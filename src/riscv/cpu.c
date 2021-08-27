#include "riscv.h"

uint32_t x[NREG];
uint32_t csr[NCSR];

char *r_name[] = {"x0", "ra", "sp", "gp", "tp", "t0", "t1", "t2", 
                  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", 
                  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", 
                  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
                  };
char *csr_name[NCSR];

#define R(i) r_name[i]
#define CSR(id) csr_name[id]
#define CSR_DEF(id, name) csr_name[id]=name

uint32_t inst;
uint32_t op, rd, rs1, rs2, funct3, funct7;
uint32_t i_imm, b_imm, u_imm, j_imm, s_imm, csr_imm, csr_id;

const char *i_op1[] = { "add", "sll",  "slt",    "sltu",  "xor", "srl",  "or",   "and"  };
const char *i_op2[] = { "sub", "?",    "?",      "?",     "?",   "sra",  "?",    "?"  };
const char *m_op[]  = { "mul", "mulh", "mulhsh", "mulhu", "div", "divu", "rem",  "remu" };
const char *b_op[]  = { "beq", "bne",  "?",      "?",     "blt", "bge",  "bltu", "bgeu" };
const char *l_op[]  = { "lb",  "lh",   "lw",     "?",     "lbu", "lhu",  "?",    "?"  }; 
const char *s_op[]  = { "sb",  "sh",   "sw",     "?",     "?",   "?",    "?",    "?"};
const char *csr_op[] = { "?", "csrrw", "csrrs", "csrrc", "?", "csrrwi", "csrrsi","csrrci" };

bool rv_init_csr() {
    memset(csr_name, 0, sizeof(char*)*NCSR);
    // CSR 列表參考 https://people.eecs.berkeley.edu/~krste/papers/riscv-privileged-v1.9.1.pdf 2.2 CSR Listing
    // The RISC-V Instruction Set Manual, Volume II: Privileged Architecture, Privileged Architecture Version 1.9.1
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

bool rv_decode(uint32_t inst) {
    op = (inst & INST_6_2) >> 2;
    rd = dec_rd(inst);
    funct3 = dec_funct3(inst);
    rs1 = csr_imm = dec_rs1(inst);
    rs2 = dec_rs2(inst);
    funct7 = dec_funct7(inst);
    i_imm = dec_itype_imm(inst);
    b_imm = dec_btype_imm(inst);
    u_imm = dec_utype_imm(inst);
    j_imm = dec_jtype_imm(inst);
    s_imm = dec_stype_imm(inst);
    csr_id = dec_csr(inst);
}

bool rv_dasm_inst(uint32_t inst, char *dasm, uint32_t pc) {
    char *cmd;
    switch (op) {
        case 0b01101: sprintf(dasm, "lui %s,0x%x", R(rd), u_imm); break;  // U-Type: lui a5,0x10000
        case 0b00101: sprintf(dasm, "auipc %s,0x%x", R(rd), u_imm); break;  // U-Type: auipc a0,0x0
        case 0b11011: sprintf(dasm, "jal %s,0x%x", R(rd), pc+j_imm); break;  // J-Type: jal ra,80008028 <lib_putc>
        case 0b11001: sprintf(dasm, "jalr %s,0x%x", R(rd), u_imm); break;  // I-Type:  JALR rd, rs1, imm
        case 0b01100:
            switch (funct7) {
              case 0b0000000: sprintf(dasm, "%s %s,%s,%s", i_op1[funct3], R(rd), R(rs1), R(rs2)); break; // R-Type: add sp,sp,t0        
              case 0b0100000: sprintf(dasm, "%s %s,%s,%s", i_op2[funct3], R(rd), R(rs1), R(rs2)); break; // R-Type: sub sp,sp,t0
              case 0b0000001: sprintf(dasm, "%s %s,%s,%s", m_op[funct3], R(rd), R(rs1), R(rs2)); break; // rv32m : mul sp,sp,t0
              default: sprintf(dasm, "?"); break;       
            }
            break;
        case 0b00100: sprintf(dasm, "%-si %s,%s,%d", i_op1[funct3], R(rd), R(rs1), i_imm); break; // I-Type: addi sp,sp,32
        case 0b11000: sprintf(dasm, "%s %s,%s,0x%x", b_op[funct3], R(rs2), R(rs1), pc+b_imm); break;  // B-Type: bne zero, a0,80000020
        case 0b00000: sprintf(dasm, "%s %s,%d(%s)", l_op[funct3], R(rd), i_imm, R(rs1)); break;  // I-type: lw a5,-20(s0)
        case 0b01000: sprintf(dasm, "%s %s,%d(%s)", s_op[funct3], R(rs2), s_imm, R(rs1)); break;  // S-Type: sw s0,8(sp)
        case 0b11100: 
          if (funct3 == 0 && rd==0 && rs1==0) {
            cmd = "?";
            switch (i_imm) {
                case 0b000000000000: cmd="ecall"; break;
                case 0b000000000001: cmd="ebreak"; break;
                case 0b000000000010: cmd="uret"; break;
                case 0b000100000010: cmd="sret"; break;
                case 0b001100000010: cmd="mret"; break;
                case 0b000100000101: cmd="wfi"; break;
            }
            sprintf(dasm, cmd);
          } else if (funct3<=3) {
            sprintf(dasm, "%s %s,%s,%s", csr_op[funct3], R(rd), R(rs1), CSR(csr_id));
          } else if (funct3>=5) {
            sprintf(dasm, "%s %s,%x,%s", csr_op[funct3], R(rd), csr_imm, CSR(csr_id));
          }
          break;
        case 0b00011:
          if (funct3 == 0) {
            uint32_t pred = i_imm&0b111, succ  = (i_imm>>3)&0b111;
            sprintf(dasm, "fence %d, %d", pred, succ); // fence pred, succ
          } else if (funct3 == 1) {
            sprintf(dasm, "fence.i");
          }
          break;
        default: sprintf(dasm, "?");
    }
    return true;
}
