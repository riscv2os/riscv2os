#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

// 陷阱：中斷的相關函數

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr(); // 外部裝置中斷

void
trapinit(void)
{
  initlock(&tickslock, "time"); // 本模組的鎖
}

// set up to take exceptions and traps while in the kernel.
void
trapinithart(void)
{
  w_stvec((uint64)kernelvec); // 設定核心的中斷函數為 kernelvec
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void
usertrap(void) // 使用者中斷 (自陷 trap)
{
  int which_dev = 0;

  if((r_sstatus() & SSTATUS_SPP) != 0) // 是否來自 user mode ?
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec); // 設定中斷向量為 kernelvec ?

  struct proc *p = myproc(); // 取得目前 process
  
  // save user program counter.
  p->trapframe->epc = r_sepc(); // 儲存程式計數器 sepc
  
  if(r_scause() == 8){ // 如果是系統呼叫
    // system call

    if(p->killed) // 如果行程已經被殺死
      exit(-1);

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4; // 得儲存 ecall 的下一個指令

    // an interrupt will change sstatus &c registers,
    // so don't enable until done with those registers.
    intr_on(); // 允許裝置中斷

    syscall(); // 執行系統呼叫
  } else if((which_dev = devintr()) != 0){ // 如果是裝置中斷，呼叫 devintr() 處理之。
    // ok
  } else { // 否則，錯誤處理
    printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
    printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    p->killed = 1;
  }

  if(p->killed)
    exit(-1);

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2) // 如果是時間中斷，禮讓給其他 process
    yield();

  usertrapret(); // 從 usertrap 返回
}

//
// return to user space
//
void
usertrapret(void)
{
  struct proc *p = myproc();

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  intr_off(); // 禁止裝置中斷

  // send syscalls, interrupts, and exceptions to trampoline.S
  w_stvec(TRAMPOLINE + (uservec - trampoline));
  // 保存 kernel 的相關暫存器
  // set up trapframe values that uservec will need when
  // the process next re-enters the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table (核心分頁表)
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack (核心堆疊)
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp();         // hartid for cpuid()

  // set up the registers that trampoline.S's sret will use
  // to get to user space.
  // 回復到使用者模式
  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc); // 設定 sepc 準備返回

  // tell trampoline.S the user page table to switch to.
  uint64 satp = MAKE_SATP(p->pagetable); // 切換回使用者分頁表

  // jump to trampoline.S at the top of memory, which 
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64 fn = TRAMPOLINE + (userret - trampoline); // 呼叫 trampoline.S 切換回使用者行程。
  ((void (*)(uint64,uint64))fn)(TRAPFRAME, satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void 
kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();
  
  if((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if(intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if((which_dev = devintr()) == 0){ // 1. 裝置中斷
    printf("scause %p\n", scause);
    printf("sepc=%p stval=%p\n", r_sepc(), r_stval());
    panic("kerneltrap");
  }

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)  // 1. 時間中斷，禮讓給別人
    yield(); // 註：時間中斷對 user mode 與 kernel mode 都是有效的，都必須禮讓給別人。

  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}

void
clockintr() // 時間中斷
{
  acquire(&tickslock);
  ticks++;
  wakeup(&ticks); // 喚醒等待時間中斷的行程
  release(&tickslock);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int
devintr() // 裝置中斷
{
  uint64 scause = r_scause();

  if((scause & 0x8000000000000000L) &&
     (scause & 0xff) == 9){ // 1. 硬體外部中斷
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if(irq == UART0_IRQ){ // UART 中斷
      uartintr();
    } else if(irq == VIRTIO0_IRQ){ // 磁碟中斷
      virtio_disk_intr();
    } else if(irq){
      printf("unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if(irq)
      plic_complete(irq); // 可以允許再次中斷了。

    return 1;
  } else if(scause == 0x8000000000000001L){ // 2. 時間中斷
    // software interrupt from a machine-mode timer interrupt,
    // forwarded by timervec in kernelvec.S.

    if(cpuid() == 0){
      clockintr();
    }
    
    // acknowledge the software interrupt by clearing
    // the SSIP bit in sip.
    w_sip(r_sip() & ~2); // 註：sip 是 software interrupt-pending

    return 2;
  } else {
    return 0;
  }
}

