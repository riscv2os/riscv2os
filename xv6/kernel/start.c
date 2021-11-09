#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

// C 語言啟動點：從 entry.S 啟動後，由 start.c 呼叫 main()
void main();
void timerinit();

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// a scratch area per CPU for machine-mode timer interrupts.
uint64 timer_scratch[NCPU][5];

// assembly code in kernelvec.S for machine-mode timer interrupt.
extern void timervec();

// entry.S jumps here in machine mode on stack0.
void
start() // 必須先設定處理器相關參數 (CSR 暫存器)
{
  // set M Previous Privilege mode to Supervisor, for mret.
  unsigned long x = r_mstatus(); // 讓 mret 之後會進入 supervisor 模式
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  w_mstatus(x);

  // set M Exception Program Counter to main, for mret.
  // requires gcc -mcmodel=medany 
  //   medany/medlow 等選項之意義請參考 -- https://www.sifive.com/blog/all-aboard-part-4-risc-v-code-models
  w_mepc((uint64)main); // mret 後會跳入 main

  // disable paging for now.
  w_satp(0); // 暫時不用分頁機制

  // delegate all interrupts and exceptions to supervisor mode.
  // 參考 -- https://www.five-embeddev.com/riscv-isa-manual/latest/machine.html#machine-trap-delegation-registers-medeleg-and-mideleg
  w_medeleg(0xffff); // medeleg: machine exception delegation register 全設定為 1，由 S-mode 處理 exception
  w_mideleg(0xffff); // mideleg: machine interrupt delegation register 全設定為 1，由 S-mode 處理 interrupt

  // sie 參考 -- https://www.five-embeddev.com/riscv-isa-manual/latest/supervisor.html#supervisor-interrupt-registers-sip-and-sie
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE); // S-mode Interrupt-Enable
  // SEIE: supervisor-level external interrupts
  // STIE: supervisor-level timer interrupts
  // SIE_SSIE: supervisor-level software interrupts

  // ask for clock interrupts.
  timerinit(); // 設定並啟動時間中斷

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  w_tp(id); // tp 是一般暫存器，速度較快，專門用來儲存 hartid，這樣不用讀取 CSR 的 hartid

  // switch to supervisor mode and jump to main().
  asm volatile("mret");
}

// set up to receive timer interrupts in machine mode,
// which arrive at timervec in kernelvec.S,
// which turns them into software interrupts for
// devintr() in trap.c.
void
timerinit() // 設定並啟動時間中斷
{
  // each CPU has a separate source of timer interrupts.
  int id = r_mhartid();

  // ask the CLINT for a timer interrupt.
  int interval = 1000000; // cycles; about 1/10th second in qemu.
  *(uint64*)CLINT_MTIMECMP(id) = *(uint64*)CLINT_MTIME + interval; // 設定下次中斷時間

  // prepare information in scratch[] for timervec.
  // scratch[0..2] : space for timervec to save registers.
  // scratch[3] : address of CLINT MTIMECMP register.
  // scratch[4] : desired interval (in cycles) between timer interrupts.
  uint64 *scratch = &timer_scratch[id][0]; // 設定 scratch 暫存區，[0..2] 保存 a1, a2, a3 等暫存器
  scratch[3] = CLINT_MTIMECMP(id); // 放入 CLINT_MTIMECMP 到 scratch[3] 給 timervec 使用
  scratch[4] = interval;           // 放入 interval 到 scratch[4] 給 timervec 使用
  w_mscratch((uint64)scratch);     // 寫入到 msratch 暫存器

  // set the machine-mode trap handler.
  w_mtvec((uint64)timervec);       // 設定時間中斷函數 (M-mode)

  // enable machine-mode interrupts.
  w_mstatus(r_mstatus() | MSTATUS_MIE); // 啟動中斷 (M-mode)

  // enable machine-mode timer interrupts.
  w_mie(r_mie() | MIE_MTIE); // 啟動時間中斷 (M-mode)
}
