// Physical memory allocator, for user processes, (分配實體記憶體區塊)
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run { // 用來管理自由區塊的鏈表 (freelist)
  struct run *next;
};

struct { // 本檔案 kalloc.c 的主要資料結構
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit() // 將全部實體記憶體區塊納入管理
{
  initlock(&kmem.lock, "kmem"); // 設定鎖
  freerange(end, (void*)PHYSTOP); // 將 end 到 PHYSTOP 的記憶體範圍納入管理
}

void
freerange(void *pa_start, void *pa_end) // 將 (pa_start, pa_end) 之間的記憶體區塊納入管理
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start); // 取得開頭區塊起始位址
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) // 一頁一頁納入自由鏈表管理
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa) // 將 pa 對應頁納入自由鏈表管理
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE); // 整塊填入特定樣式 0x01 

  // 以下程式讓本頁 pa 成為新的鏈表頭  (pa->next = freelist)
  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist; 
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void) // 分配一頁尚未被使用的實體記憶體
{
  struct run *r;

  acquire(&kmem.lock);

  // 以下程式將第一塊分配出去，並將 freelist 指向第二塊
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
 