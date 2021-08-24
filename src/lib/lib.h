#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define ERROR(...) { fprintf(stderr, "Error:"); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); exit(1); }

#define max(a,b) ((a)>(b))?(a):(b)
#define min(a,b) ((a)<(b))?(a):(b)
