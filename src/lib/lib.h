#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define error() assert(0)

#define max(a,b) ((a)>(b))?(a):(b)
#define min(a,b) ((a)<(b))?(a):(b)
