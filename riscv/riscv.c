#include "riscv.h"

/*
    INST_6_2
    // clang-format off
    TABLE_TYPE jump_table[] = {
    //  000         001           010        011           100         101        110   111
        OP(load),   OP(load_fp),  OP(unimp), OP(misc_mem), OP(op_imm), OP(auipc), OP(unimp), OP(unimp), // 00
        OP(store),  OP(store_fp), OP(unimp), OP(amo),      OP(op),     OP(lui),   OP(unimp), OP(unimp), // 01
        OP(madd),   OP(msub),     OP(nmsub), OP(nmadd),    OP(fp),     OP(unimp), OP(unimp), OP(unimp), // 10
        OP(branch), OP(jalr),     OP(unimp), OP(jal),      OP(system), OP(unimp), OP(unimp), OP(unimp), // 11
    };
*/

static bool rv_dasm_rtype(uint32_t inst, char *str, uint32_t group) {
    const uint32_t rd = dec_rd(inst);
    const uint32_t funct3 = dec_funct3(inst);
    const uint32_t rs1 = dec_rs1(inst);
    const uint32_t rs2 = dec_rs2(inst);
    const uint32_t funct7 = dec_funct7(inst);

    char imm = (group == 0b00100)?'i':' ';
    char *op = "???";
    char *i_op[] = { "add", "sll", "slt", "sltu", "xor", "srl", "or", "and" }; 
    char *m_op[] = { "mul", "mulh", "mulhsh", "mulhu", "div", "divu", "rem", "remu" };
    switch (funct7) {
    case 0b0000000: // RV32I instructions
        op = i_op[funct3];
        break;
    case 0b0000001: // RV32M instructions
        op = m_op[funct3];
        break;
    case 0b0100000: // RV32I instructions
        switch (funct3) {
        case 0b000:  // SUB
            op = "sub";
            break;
        case 0b101:  // SRA
            op = "sra";
            break;
        }
    }
    sprintf(str, "%5s%c %d %d %d", op, imm, rd, rs1, rs2);
}

static bool rv_dasm_itype(uint32_t inst, char *str, uint32_t group) {
}

static bool rv_dasm_stype(uint32_t inst, char *str, uint32_t group) {
}

static bool rv_dasm_btype(uint32_t inst, char *str, uint32_t group) {
}

static bool rv_dasm_utype(uint32_t inst, char *str, uint32_t group) {
}

static bool rv_dasm_jtype(uint32_t inst, char *str, uint32_t group) {
}

static bool rv_dasm(uint32_t inst, char *str)
{
    uint32_t group = (inst & INST_6_2) >> 2;
    switch (group) {
        case 0b01100: // r-type
        case 0b00100:
            rv_dasm_rtype(inst, str, group);
            break;
        /*
        case : // i-type
            rv_dasm_itype(inst, str, group);
            break;
        case : // s-type
            rv_dasm_stype(inst, str, group);
            break;
        case : // i-type
            rv_dasm_btype(inst, str, group);
            break;
        case : // i-type
            rv_dasm_utype(inst, str, group);
            break;
        case : // i-type
            rv_dasm_jtype(inst, str, group);
            break;
        */
        default:
            sprintf(str, "xxx");
    }
    printf("group=%05x ", group);
    return true;
}

bool rv_decode(uint8_t *code, int addr, int size) {
    char line[256];
    for (uint8_t *p = code; p<code+size; p += sizeof(uint32_t)) {
        // uint32_t ir = p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
        uint32_t ir = * (uint32_t*) p;
        if (ir == 0) continue;
        rv_dasm(ir, line);
        printf("%09x: %08x %s\n", addr+p-code, ir, line);
    }
}
