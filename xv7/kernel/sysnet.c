//
// network system calls.
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "net.h"

struct sock {
  struct sock *next; // the next socket in the list
  uint32 raddr;      // the remote IPv4 address
  uint16 lport;      // the local UDP port number
  uint16 rport;      // the remote UDP port number
  struct spinlock lock; // protects the rxq
  struct mbufq rxq;  // a queue of packets waiting to be received
};

static struct spinlock lock;
static struct sock *sockets;

void
sockinit(void)
{
  initlock(&lock, "socktbl");
}

int
sockalloc(struct file **f, uint32 raddr, uint16 lport, uint16 rport)
{
  struct sock *si, *pos;

  si = 0;
  *f = 0;
  if ((*f = filealloc()) == 0)
    goto bad;
  if ((si = (struct sock*)kalloc()) == 0)
    goto bad;

  // initialize objects
  si->raddr = raddr;
  si->lport = lport;
  si->rport = rport;
  initlock(&si->lock, "sock");
  mbufq_init(&si->rxq);
  (*f)->type = FD_SOCK;
  (*f)->readable = 1;
  (*f)->writable = 1;
  (*f)->sock = si;

  // add to list of sockets
  acquire(&lock);
  pos = sockets;
  while (pos) {
    if (pos->raddr == raddr &&
        pos->lport == lport &&
	      pos->rport == rport) {
      release(&lock);
      goto bad;
    }
    pos = pos->next;
  }
  si->next = sockets;
  sockets = si;
  release(&lock);
  return 0;

bad:
  if (si)
    kfree((char*)si);
  if (*f)
    fileclose(*f);
  return -1;
}

void
sockclose(struct sock *so) {
  acquire(&so->lock);
  wakeup(&so->rxq);
  release(&so->lock);

  acquire(&lock);
  struct sock *sop = sockets;
  if(sop->next == 0)
    sockets = 0;
  while (sop && sop->next) {
    if (sop->next->raddr == so->raddr &&
        sop->next->lport == so->lport &&
	      sop->next->rport == so->rport) {
      sop->next = so->next;
      break;
    }
    sop = sop->next;
  }
  release(&lock);

  kfree((char*)so);
}

int
sockwrite(struct sock *so, uint64 addr, int n) {
  struct proc *p = myproc();

  struct mbuf *m = mbufalloc(sizeof(struct eth) + sizeof(struct ip) + sizeof(struct udp));

  mbufput(m, n);

  if(copyin(p->pagetable, m->head, addr, n) == -1) {
    mbuffree(m);
    return -1;
  }

  net_tx_udp(m, so->raddr, so->lport, so->rport);

  return n;
}

int
sockread(struct sock *so, uint64 addr, int n) {
  struct proc *p = myproc();

  acquire(&so->lock);

  if(mbufq_empty(&so->rxq)) {
    while (mbufq_empty(&so->rxq))
      sleep(&so->rxq, &so->lock);
  }
  
  struct mbuf *m = mbufq_pophead(&so->rxq);
  int len = n < m->len ? n : m->len;

  if(copyout(p->pagetable, addr, m->head, len) == -1) {
    release(&so->lock);
    mbuffree(m);
    return -1;
  }

  release(&so->lock);
  mbuffree(m);
  return len;
}

// called by protocol handler layer to deliver UDP packets
void
sockrecvudp(struct mbuf *m, uint32 raddr, uint16 lport, uint16 rport) {
  struct sock *so = sockets;
  acquire(&lock);
  while(so) {
    if (so->raddr == raddr &&
        so->lport == lport &&
	      so->rport == rport) {
      break;
    }
    so = so->next;
  }
  release(&lock);
  
  // if no socket machted
  if(so == 0) {
    mbuffree(m);
    return;
  }

  acquire(&so->lock);
  mbufq_pushtail(&so->rxq, m);
  release(&so->lock);
  wakeup(&so->rxq);
}
