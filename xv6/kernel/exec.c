#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "elf.h"

static int loadseg(pde_t *pgdir, uint64 addr, struct inode *ip, uint offset, uint sz); // 載入分段

int
exec(char *path, char **argv) // 將行程換成 path 指定的執行檔。
{
  char *s, *last;
  int i, off;
  uint64 argc, sz = 0, sp, ustack[MAXARG+1], stackbase;
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pagetable_t pagetable = 0, oldpagetable;
  struct proc *p = myproc();

  begin_op(); // 使用檔案系統前需要呼叫 log.c 中的 begin_op() ，這樣才會記錄存取動作到日誌中

  if((ip = namei(path)) == 0){ // 取得 path ELF 檔對應的 inode ptr (ip)
    end_op();
    return -1;
  }
  ilock(ip); // 鎖定該 inode

  // Check ELF header
  if(readi(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf)) // 讀取該 inode
    goto bad;
  if(elf.magic != ELF_MAGIC) // 若不是 ELF 則失敗
    goto bad;

  if((pagetable = proc_pagetable(p)) == 0) // 創建分頁表
    goto bad;

  // Load program into memory.
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){ // 對於 ELF 的每一段
    if(readi(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph)) // 讀入該段
      goto bad;
    if(ph.type != ELF_PROG_LOAD) // 只處理 LOAD 類型的段
      continue;
    if(ph.memsz < ph.filesz) // 記憶體大小一定大於檔案大小 (一般是相等，但 BSS 段則是 memsz > filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr) // 溢位 => 失敗
      goto bad;
    uint64 sz1;
    if((sz1 = uvmalloc(pagetable, sz, ph.vaddr + ph.memsz)) == 0) // 為每個ELF段分配記憶體
      goto bad;
    sz = sz1;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loadseg(pagetable, ph.vaddr, ip, ph.off, ph.filesz) < 0) // 把每個段加載到 ph.vaddr 記憶體中
      goto bad; 
  }
  iunlockput(ip); // 載入完成，解除 inode 鎖定
  end_op(); // 使用完檔案系統前需要呼叫 log.c 中的 end_op() 
  ip = 0;

  p = myproc();
  uint64 oldsz = p->sz;

  // Allocate two pages at the next page boundary. 為何分配兩頁？第二個是堆疊，那第一個幹嘛用？
  // Use the second as the user stack.  答：第一個是不可訪問頁，當堆疊溢位時會觸發錯誤中斷。
  sz = PGROUNDUP(sz); // 讓 sz = n 個完整分頁大小
  uint64 sz1;
  if((sz1 = uvmalloc(pagetable, sz, sz + 2*PGSIZE)) == 0) // 多分配兩頁，用途如上所述
    goto bad;
  sz = sz1;
  uvmclear(pagetable, sz-2*PGSIZE); // 清除分頁內容
  sp = sz;
  stackbase = sp - PGSIZE; // 堆疊在最後一頁

  // Push argument strings, prepare rest of stack in ustack. (在堆疊中推入 argv 字串)
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp -= strlen(argv[argc]) + 1;
    sp -= sp % 16; // riscv sp must be 16-byte aligned (對齊 sp)
    if(sp < stackbase)
      goto bad;
    if(copyout(pagetable, sp, argv[argc], strlen(argv[argc]) + 1) < 0) // 複製到使用者行程，失敗就離開
      goto bad;
    ustack[argc] = sp;
  }
  ustack[argc] = 0;

  // push the array of argv[] pointers. (推入 argv 的指標)
  sp -= (argc+1) * sizeof(uint64);
  sp -= sp % 16; // 對齊
  if(sp < stackbase)
    goto bad;
  if(copyout(pagetable, sp, (char *)ustack, (argc+1)*sizeof(uint64)) < 0) // 複製到使用者行程，失敗就離開
    goto bad;

  // arguments to user main(argc, argv)
  // argc is returned via the system call return
  // value, which goes in a0.
  p->trapframe->a1 = sp; // 設定 a1=argv

  // Save program name for debugging. (儲存程式名稱，除錯用)
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(p->name, last, sizeof(p->name));
    
  // Commit to the user image.
  oldpagetable = p->pagetable; // oldpagetable 指向 fork 時的舊行程頁表
  p->pagetable = pagetable;    // 指向新的分頁表
  p->sz = sz;
  p->trapframe->epc = elf.entry;  // initial program counter = main (進入點 epc 為 elf.entry)
  p->trapframe->sp = sp; // initial stack pointer (設定 p 的堆疊暫存器)
  proc_freepagetable(oldpagetable, oldsz); // 將舊的行程頁表釋放

  return argc; // this ends up in a0, the first argument to main(argc, argv)

 bad: // 錯誤處理
  if(pagetable)
    proc_freepagetable(pagetable, sz);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}

// loadseg用walkaddr找到分配的內存的物理地址，在這個地址上寫入ELF段的每一頁，用readi從文件中讀取
// Load a program segment into pagetable at virtual address va.
// va must be page-aligned
// and the pages from va to va+sz must already be mapped.
// Returns 0 on success, -1 on failure.
static int
loadseg(pagetable_t pagetable, uint64 va, struct inode *ip, uint offset, uint sz)
{
  uint i, n;
  uint64 pa;

  if((va % PGSIZE) != 0)
    panic("loadseg: va must be page aligned");

  for(i = 0; i < sz; i += PGSIZE){
    pa = walkaddr(pagetable, va + i); // 取得虛擬位址 va 對應的實體位址 pa
    if(pa == 0)
      panic("loadseg: address should exist");
    if(sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;
    if(readi(ip, 0, (uint64)pa, offset+i, n) != n) // 從磁碟讀入該段
      return -1;
  }
  
  return 0;
}
