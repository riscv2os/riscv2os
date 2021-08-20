#include "riscv.h"

char *r_name[] = {"x0", "ra", "sp", "gp", "tp", "t0", "t1", "t2", 
                  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", 
                  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", 
                  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
                  };
const char *i_op[] = { "add", "sll",  "slt",    "sltu",  "xor", "srl",  "or",   "and"  }; 
const char *m_op[] = { "mul", "mulh", "mulhsh", "mulhu", "div", "divu", "rem",  "remu" };
const char *b_op[] = { "beq", "bne",  "?",      "?",     "blt", "bge",  "bltu", "bgeu" };
const char *l_op[] = { "lb",  "lh",   "lw",     "?",     "lbu", "lhu",  "?",    "?"  }; 
const char *s_op[] = { "sb",  "sh",   "sw",     "?",     "?",   "?",    "?",    "?"};
const char *csr_op[] = { "?", "csrrw", "csrrs", "csrrc", "?", "csrrwi", "csrrsi", "csrrci" };

#define R(i) r_name[i]
#define CSR(id) csr_name[id]

uint32_t op, rd, rs1, rs2, funct3, funct7;
uint32_t i_imm, b_imm, u_imm, j_imm, s_imm, csr_imm, csr;

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
    csr = dec_csr(inst);
}

bool rv_dasm(uint32_t inst, char *dasm, uint32_t pc) {
    char *cmd;
    switch (op) {
        case 0b01101: sprintf(dasm, "lui %s,0x%x", R(rd), u_imm); break;  // U-Type: lui a5,0x10000
        case 0b00101: sprintf(dasm, "auipc %s,0x%x", R(rd), u_imm); break;  // U-Type: auipc a0,0x0
        case 0b11011: sprintf(dasm, "jal %s,0x%x", R(rd), pc+j_imm); break;  // J-Type: jal ra,80008028 <lib_putc>
        case 0b11001: sprintf(dasm, "jalr %s,0x%x", R(rd), u_imm); break;  // I-Type:  JALR rd, rs1, imm
        case 0b01100: sprintf(dasm, "%s %s,%s,%s", i_op[funct3], R(rd), R(rs1), R(rs2)); break; // R-Type: add sp,sp,t0
        case 0b00100: sprintf(dasm, "%-si %s,%s,%d", i_op[funct3], R(rd), R(rs1), i_imm); break; // I-Type: addi sp,sp,32
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
            sprintf(dasm, "%s %s,%s,%s", csr_op[funct3], R(rd), R(rs1), CSR(csr));
          } else if (funct3>=5) {
            sprintf(dasm, "%s %s,%x,%s", csr_op[funct3], R(rd), csr_imm, CSR(csr));
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
        default:
            sprintf(dasm, "?");
    }
    return true;
}
