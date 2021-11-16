#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

// Simple logging that allows concurrent FS system calls.
//
// A log transaction contains the updates of multiple FS system
// calls. The logging system only commits when there are
// no FS system calls active. Thus there is never
// any reasoning required about whether a commit might
// write an uncommitted system call's updates to disk.
//
// A system call should call begin_op()/end_op() to mark
// its start and end. Usually begin_op() just increments
// the count of in-progress FS system calls and returns.
// But if it thinks the log is close to running out, it
// sleeps until the last outstanding end_op() commits.
//
// The log is a physical re-do log containing disk blocks.
// The on-disk log format:
//   header block, containing block #s for block A, B, C, ...
//   block A
//   block B
//   block C
//   ...
// Log appends are synchronous.

// Contents of the header block, used for both the on-disk header block
// and to keep track in memory of logged block# before commit.
struct logheader {
  int n;
  int block[LOGSIZE];
};

struct log {
  struct spinlock lock;
  int start;
  int size;
  int outstanding; // how many FS sys calls are executing.
  int committing;  // in commit(), please wait.
  int dev;
  struct logheader lh;
};
struct log log[NDISK];

static void recover_from_log(int);
static void commit(int);

void
initlog(int dev, struct superblock *sb)
{
  if (sizeof(struct logheader) >= BSIZE)
    panic("initlog: too big logheader");

  initlock(&log[dev].lock, "log");
  log[dev].start = sb->logstart;
  log[dev].size = sb->nlog;
  log[dev].dev = dev;
  recover_from_log(dev);
}

// Copy committed blocks from log to their home location
static void
install_trans(int dev)
{
  int tail;

  for (tail = 0; tail < log[dev].lh.n; tail++) {
    struct buf *lbuf = bread(dev, log[dev].start+tail+1); // read log block
    struct buf *dbuf = bread(dev, log[dev].lh.block[tail]); // read dst
    memmove(dbuf->data, lbuf->data, BSIZE);  // copy block to dst
    bwrite(dbuf);  // write dst to disk
    bunpin(dbuf);
    brelse(lbuf);
    brelse(dbuf);
  }
}

// Read the log header from disk into the in-memory log header
static void
read_head(int dev)
{
  struct buf *buf = bread(dev, log[dev].start);
  struct logheader *lh = (struct logheader *) (buf->data);
  int i;
  log[dev].lh.n = lh->n;
  for (i = 0; i < log[dev].lh.n; i++) {
    log[dev].lh.block[i] = lh->block[i];
  }
  brelse(buf);
}

// Write in-memory log header to disk.
// This is the true point at which the
// current transaction commits.
static void
write_head(int dev)
{
  struct buf *buf = bread(dev, log[dev].start);
  struct logheader *hb = (struct logheader *) (buf->data);
  int i;
  hb->n = log[dev].lh.n;
  for (i = 0; i < log[dev].lh.n; i++) {
    hb->block[i] = log[dev].lh.block[i];
  }
  bwrite(buf);
  brelse(buf);
}

static void
recover_from_log(int dev)
{
  read_head(dev);
  install_trans(dev); // if committed, copy from log to disk
  log[dev].lh.n = 0;
  write_head(dev); // clear the log
}

// called at the start of each FS system call.
void
begin_op(int dev)
{
  acquire(&log[dev].lock);
  while(1){
    if(log[dev].committing){
      sleep(&log, &log[dev].lock);
    } else if(log[dev].lh.n + (log[dev].outstanding+1)*MAXOPBLOCKS > LOGSIZE){
      // this op might exhaust log space; wait for commit.
      sleep(&log, &log[dev].lock);
    } else {
      log[dev].outstanding += 1;
      release(&log[dev].lock);
      break;
    }
  }
}

// called at the end of each FS system call.
// commits if this was the last outstanding operation.
void
end_op(int dev)
{
  int do_commit = 0;

  acquire(&log[dev].lock);
  log[dev].outstanding -= 1;
  if(log[dev].committing)
    panic("log[dev].committing");
  if(log[dev].outstanding == 0){
    do_commit = 1;
    log[dev].committing = 1;
  } else {
    // begin_op() may be waiting for log space,
    // and decrementing log[dev].outstanding has decreased
    // the amount of reserved space.
    wakeup(&log);
  }
  release(&log[dev].lock);

  if(do_commit){
    // call commit w/o holding locks, since not allowed
    // to sleep with locks.
    commit(dev);
    acquire(&log[dev].lock);
    log[dev].committing = 0;
    wakeup(&log);
    release(&log[dev].lock);
  }
}

// Copy modified blocks from cache to log.
static void
write_log(int dev)
{
  int tail;

  for (tail = 0; tail < log[dev].lh.n; tail++) {
    struct buf *to = bread(dev, log[dev].start+tail+1); // log block
    struct buf *from = bread(dev, log[dev].lh.block[tail]); // cache block
    memmove(to->data, from->data, BSIZE);
    bwrite(to);  // write the log
    brelse(from);
    brelse(to);
  }
}

static void
commit(int dev)
{
  if (log[dev].lh.n > 0) {
    write_log(dev);     // Write modified blocks from cache to log
    write_head(dev);    // Write header to disk -- the real commit
    install_trans(dev); // Now install writes to home locations
    log[dev].lh.n = 0;
    write_head(dev);    // Erase the transaction from the log
  }
}

// Caller has modified b->data and is done with the buffer.
// Record the block number and pin in the cache by increasing refcnt.
// commit()/write_log() will do the disk write.
//
// log_write() replaces bwrite(); a typical use is:
//   bp = bread(...)
//   modify bp->data[]
//   log_write(bp)
//   brelse(bp)
void
log_write(struct buf *b)
{
  int i;

  int dev = b->dev;
  if (log[dev].lh.n >= LOGSIZE || log[dev].lh.n >= log[dev].size - 1)
    panic("too big a transaction");
  if (log[dev].outstanding < 1)
    panic("log_write outside of trans");

  acquire(&log[dev].lock);
  for (i = 0; i < log[dev].lh.n; i++) {
    if (log[dev].lh.block[i] == b->blockno)   // log absorbtion
      break;
  }
  log[dev].lh.block[i] = b->blockno;
  if (i == log[dev].lh.n) {  // Add new block to log?
    bpin(b);
    log[dev].lh.n++;
  }
  release(&log[dev].lock);
}


