#include "trap.h"

extern void os_kernel();

// a scratch area per CPU for machine-mode timer interrupts.
reg_t trap_scratch[NCPU][7];
enum { M_TIME_CMP=3, INTERVAL, M_EPC, M_CAUSE };

enum { M_SOFTINT=3, M_TIME_INT=7, M_EXT_INT=11 };
// scratch[id][0..2] : space for timervec to save registers.
// scratch[id][3] : address of CLINT MTIMECMP register.
// scratch[id][4] : desired interval (in cycles) between timer interrupts.
// scratch[id][5] : mepc
// scratch[id][6] : mcause

// ======================  ======================

#define interval 20000000 // cycles; about 2 second in qemu.


// ====================== Timer ======================
void trap_init()
{
  // each CPU has a separate source of timer interrupts.
  int id = r_mhartid();

  // ask the CLINT for a timer interrupt.
  *(reg_t*)CLINT_MTIMECMP(id) = *(reg_t*)CLINT_MTIME + interval;

  // prepare information in scratch[] for timervec.
  reg_t *scratch = &trap_scratch[id][0];
  scratch[3] = CLINT_MTIMECMP(id);
  scratch[4] = interval;
  w_mscratch((reg_t)scratch);

  // set the machine-mode trap handler.
  w_mtvec((reg_t)sys_trap);

  // enable machine-mode interrupts.
  w_mstatus(r_mstatus() | MSTATUS_MIE);

  // enable machine-mode timer interrupts.
  w_mie(r_mie() | MIE_MTIE);
}

static int timer_count = 0;

void timer_handler() {
  lib_printf("timer_handler: %d\n", ++timer_count);
  os_kernel();
}

void external_handler()
{
  lib_printf("externel_handler\n");
  int irq = plic_claim();
  if (irq == UART0_IRQ)
  {
    if (lib_kbhit()) {
      int c = lib_getc();
      lib_putc((char)c);
      lib_putc('\n');
    }
  }
  else if (irq)
  {
    lib_printf("unexpected interrupt irq = %d\n", irq);
  }

  if (irq)
  {
    plic_complete(irq);
  }
}

void trap_handler() {
  lib_printf("trap_handler()\n");
  int id = r_mhartid();
  reg_t *scratch = &trap_scratch[id][0];
  int cause = scratch[M_CAUSE] & 0xfff; // 
  lib_printf("cause=%x\n", cause);
  switch (cause) {
    // case M_SOFTINT: break;
    case M_TIME_INT: timer_handler(); break;
    case M_EXT_INT: external_handler(); break;
    default: 
      lib_printf("Error: cause not handled!\n");
      while (1) {}
  }
  // timer_handler();
}
