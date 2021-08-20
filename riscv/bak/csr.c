#include "riscv.h"

// CSR 列表參考 https://people.eecs.berkeley.edu/~krste/papers/riscv-privileged-v1.9.1.pdf 2.2 CSR Listing
// The RISC-V Instruction Set Manual, Volume II: Privileged Architecture, Privileged Architecture Version 1.9.1
char *csr_name[NCSR];

#define CSR_DEF(id, name) csr_name[id]=name

bool csr_init() {
memset(csr_name, 0, sizeof(char*)*NCSR);
// User Trap Setup
CSR_DEF(0x000, "ustatus"); // User status register.
CSR_DEF(0x004, "uie"); // User interrupt-enable register.
CSR_DEF(0x005, "utvec"); // User trap handler base address.
// User Trap Handling
CSR_DEF(0x040, "uscratch"); // Scratch register for user trap handlers.
CSR_DEF(0x041, "uepc"); // User exception program counter.
CSR_DEF(0x042, "ucause"); // User trap cause.
CSR_DEF(0x043, "ubadaddr"); // User bad address.
CSR_DEF(0x044, "uip"); // User interrupt pending.
// User Floating-Point CSRs
CSR_DEF(0x001, "fflags"); // Floating-Point Accrued Exceptions.
CSR_DEF(0x002, "frm"); // Floating-Point Dynamic Rounding Mode.
CSR_DEF(0x003, "fcsr"); // Floating-Point Control and Status Register (frm + fflags).
// User Counter/Timers
CSR_DEF(0xC00, "cycle"); // Cycle counter for RDCYCLE instruction.
CSR_DEF(0xC01, "time"); // Timer for RDTIME instruction.
CSR_DEF(0xC02, "instret"); // Instructions-retired counter for RDINSTRET instruction.
// CSR_DEF(0xC03, "hpmcounter0-31"); // Performance-monitoring counter.
CSR_DEF(0xC80, "cycleh"); // Upper 32 bits of cycle, RV32I only.
CSR_DEF(0xC81, "timeh"); // Upper 32 bits of time, RV32I only.
CSR_DEF(0xC82, "instreth"); // Upper 32 bits of instret, RV32I only.
// CSR_DEF(0xC83, "hpmcounter3-31h Upper 32 bits of hpmcounter3, RV32I only.
// Supervisor Trap Setup
CSR_DEF(0x100, "sstatus"); //  Supervisor status register.
CSR_DEF(0x102, "sedeleg"); //  Supervisor exception delegation register.
CSR_DEF(0x103, "sideleg"); //  Supervisor interrupt delegation register.
CSR_DEF(0x104, "sie"); //  Supervisor interrupt-enable register.
CSR_DEF(0x105, "stvec"); //  Supervisor trap handler base address.
// Supervisor Trap Handling
CSR_DEF(0x140, "sscratch"); //  Scratch register for supervisor trap handlers.
CSR_DEF(0x141, "sepc"); //  Supervisor exception program counter.
CSR_DEF(0x142, "scause"); //  Supervisor trap cause.
CSR_DEF(0x143, "sbadaddr"); //  Supervisor bad address.
CSR_DEF(0x144, "sip"); //  Supervisor interrupt pending.
// Supervisor Protection and Translation
CSR_DEF(0x180, "sptbr"); //  Page-table base register.
// Hypervisor Trap Setup
CSR_DEF(0x200, "hstatus"); //  Hypervisor status register.
CSR_DEF(0x202, "hedeleg"); //  Hypervisor exception delegation register.
CSR_DEF(0x203, "hideleg"); //  Hypervisor interrupt delegation register.
CSR_DEF(0x204, "hie"); //  Hypervisor interrupt-enable register.
CSR_DEF(0x205, "htvec"); //  Hypervisor trap handler base address.
// Hypervisor Trap Setup
CSR_DEF(0x240, "hscratch"); //  Scratch register for hypervisor trap handlers.
CSR_DEF(0x241, "hepc"); //  Hypervisor exception program counter.
CSR_DEF(0x242, "hcause"); //  Hypervisor trap cause.
CSR_DEF(0x243, "hbadaddr"); //  Hypervisor bad address.
CSR_DEF(0x244, "hip"); //  Hypervisor interrupt pending.
// Hypervisor Protection and Translation .. TBD.
// Machine Information Registers
CSR_DEF(0xF11, "mvendorid"); //  Vendor ID.
CSR_DEF(0xF12, "marchid"); //  Architecture ID.
CSR_DEF(0xF13, "mimpid"); //  Implementation ID.
CSR_DEF(0xF14, "mhartid"); //  Hardware thread ID.
// Machine Trap Setup
CSR_DEF(0x300, "mstatus"); //  Machine status register.
CSR_DEF(0x301, "misa"); //  ISA and extensions
CSR_DEF(0x302, "medeleg"); //  Machine exception delegation register.
CSR_DEF(0x303, "mideleg"); //  Machine interrupt delegation register.
CSR_DEF(0x304, "mie"); //  Machine interrupt-enable register.
CSR_DEF(0x305, "mtvec"); //  Machine trap-handler base address.
// Machine Trap Handling
CSR_DEF(0x340, "mscratch"); //  Scratch register for machine trap handlers.
CSR_DEF(0x341, "mepc"); //  Machine exception program counter.
CSR_DEF(0x342, "mcause"); //  Machine trap cause.
CSR_DEF(0x343, "mbadaddr"); //  Machine bad address.
CSR_DEF(0x344, "mip"); //  Machine interrupt pending.
// Machine Protection and Translation
CSR_DEF(0x380, "mbase"); //  Base register.
CSR_DEF(0x381, "mbound"); //  Bound register.
CSR_DEF(0x382, "mibase"); //  Instruction base register.
CSR_DEF(0x383, "mibound"); //  Instruction bound register.
CSR_DEF(0x384, "mdbase"); //  Data base register.
CSR_DEF(0x385, "mdbound"); //  Data bound register
// Machine Counter/Timers
CSR_DEF(0xB00, "mcycle"); // Machine cycle counter.
CSR_DEF(0xB02, "minstret"); // Machine instructions-retired counter.
// 0xB03 MRW mhpmcounter3-31 Machine performance-monitoring counter.
// Machine Counter Setup
CSR_DEF(0x320, "mucounteren"); // User-mode counter enable.
CSR_DEF(0x321, "mscounteren"); // Supervisor-mode counter enable.
CSR_DEF(0x322, "mhcounteren"); // Hypervisor-mode counter enable.
// 0x323, mhpmevent3-31 Machine performance-monitoring event selector.
// Debug/Trace Registers (shared with Debug Mode)
CSR_DEF(0x7A0, "tselect"); // Debug/Trace trigger register select.
CSR_DEF(0x7A1, "tdata1"); // First Debug/Trace trigger data register.
CSR_DEF(0x7A2, "tdata2"); // Second Debug/Trace trigger data register.
CSR_DEF(0x7A3, "tdata3"); // Third Debug/Trace trigger data register.
// Debug Mode Registers
CSR_DEF(0x7B0, "dcsr"); // Debug control and status register.
CSR_DEF(0x7B1, "dpc"); // Debug PC.
CSR_DEF(0x7B2, "dscratch"); // Debug scratch register.
}
