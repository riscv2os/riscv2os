#include <stdio.h>
#include "../elf/elf.h"
#include "riscv.h"

static bool rv_exec(uint32_t inst, uint32_t pc)
{
    char *cmd;
    rv_decode(inst);
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

bool rv_run(char *code, int addr, int size) {
    char line[256];
    uint32_t *endp = (uint32_t *) (code + size);
    for (uint32_t *ir = (uint32_t *) code; ir<endp; ir++) {

        uint32_t pc = addr+(char*)ir-code;
        rv_exec(*ir, line, pc);
        printf("%09x %09x %s\n", pc, *ir, line);
    }
}

elf_t elf;

int main(int argc, char *argv[]) {
    char *fname=argv[1];
    if (!elf_load(&elf, fname)) {
        fprintf(stderr, "ELF file '%s' load fail \n", fname);
        return 1; 
    }
    elf_section_t s = elf_section(&elf, ".text");
    printf(".text: body=%p offset=0x%x size=%u\n", s.body, s.addr, s.size);
    csr_init();
    rv_run(s.body, s.addr, s.size);
}
