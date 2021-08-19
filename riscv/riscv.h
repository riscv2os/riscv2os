#include <stdint.h>
#include <string.h>
#include "decoder.h"

extern char *r_name[];
extern char *csr_name[];

#define R(i) r_name[i]

#define CSR_DEF(id, name) csr_name[id]=name
#define CSR(id) csr_name[id]

#define NCSR 0x1000

void csr_init();
