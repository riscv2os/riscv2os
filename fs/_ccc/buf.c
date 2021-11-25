// Return a locked buf with the contents of the indicated block.
struct buf* buf_read(uint dev, uint blockno) // 讀取 blockno 對應的區塊
{
  struct buf *b;

  b = buf_get(dev, blockno); // 取得該緩衝區塊
  if(!b->valid) { // 若還沒有從磁碟載入
    disk_rw(b, 0); // 則呼叫 virtio 模組從磁碟讀入該區塊
    b->valid = 1;
  }
  return b;
}


// Write b's contents to disk.  Must be locked.
void buf_write(struct buf *b) // 寫入緩衝區塊 b
{
  if(!lock_holding(&b->lock))
    panic("buf_write");
  disk_rw(b, 1); // 呼叫 virtio 模組寫入該區塊
}


// Release a locked buffer.
// Move to the head of the most-recently-used list.
void buf_release(struct buf *b) // 釋放緩衝區塊 b 
{
  if(!lock_holding(&b->lock))
    panic("brelease");

  lock_release(&b->lock);

  lock_acquire(&bcache.lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
  
  lock_release(&bcache.lock);
}

void buf_pin(struct buf *b) { // 增加緩衝區塊 b 的引用數
  lock_acquire(&bcache.lock);
  b->refcnt++;
  lock_release(&bcache.lock);
}

void buf_unpin(struct buf *b) { // 減少緩衝區塊 b 的引用數
  lock_acquire(&bcache.lock);
  b->refcnt--;
  lock_release(&bcache.lock);
}


// Zero a block. (將區塊清零)
static void
buf_zero(int dev, int bno)
{
  struct buf *bp;

  bp = buf_read(dev, bno); // 讀入區塊
  memset(bp->data, 0, BSIZE); // 清零
  log_write(bp); // 寫入日誌
  buf_release(bp); // 緩衝區釋放
}

// Blocks.

// Allocate a zeroed disk block.
static uint
buf_alloc(uint dev) // 分配一個空區塊
{
  int b, bi, m;
  struct buf *bp;

  bp = 0;
  for(b = 0; b < sb.size; b += BPB){
    bp = bread(dev, BBLOCK(b, sb)); // 讀取 freemap 對應 b 的區塊號
    for(bi = 0; bi < BPB && b + bi < sb.size; bi++){
      m = 1 << (bi % 8); // 算出對應的 bit 位址
      if((bp->data[bi/8] & m) == 0){  // Is block free? (若該區塊為 free)
        bp->data[bi/8] |= m;  // Mark block in use. (標示該區塊為已使用)
        log_write(bp); // 紀錄日誌
        brelse(bp); // 釋放緩衝
        bzero(dev, b + bi); // 清除該區塊
        return b + bi; // 傳回區塊號碼
      }
    }
    brelse(bp); // 釋放緩衝
  }
  panic("balloc: out of blocks");
}

// Free a disk block. (釋放磁碟區塊)
static void
buf_free(int dev, uint b)
{
  struct buf *bp;
  int bi, m;

  bp = bread(dev, BBLOCK(b, sb)); // 讀取對應的 free map 區塊
  bi = b % BPB; // 算出 freeblock 塊中位移
  m = 1 << (bi % 8); // 算出對應的 bit 位址
  if((bp->data[bi/8] & m) == 0) // 若該 bit 為 0 (代表沒人用)
    panic("freeing free block"); // 那就不應該再釋放 => 錯誤
  bp->data[bi/8] &= ~m; // 將該區塊標示為已使用
  log_write(bp); // 紀錄日誌
  brelse(bp); // 釋放緩衝
}
