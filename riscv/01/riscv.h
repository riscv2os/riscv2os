#include <stdint.h>
#include <string.h>
#include "decoder.h"

#define NREG 32
#define NCSR 0x1000

extern char *r_name[NCSR];
extern char *csr_name[NCSR];

bool csr_init();
bool rv_decode(uint32_t inst);
bool rv_dasm(uint32_t inst, char *dasm, uint32_t pc);


