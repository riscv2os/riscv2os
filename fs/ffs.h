#pragma once

#include "fs.h"

void ffs_create(char *imgFile);
int ffs_init();
int ffs_save();
int ffs_build(int fcount, char *fnames[]);
void ffs_rsect(uint sec, void *buf);
void ffs_wsect(uint sec, void *buf);
