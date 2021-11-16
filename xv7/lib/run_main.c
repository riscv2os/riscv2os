#include "lib.h"

int main(int argc, char *argv[]);

int run_main(int argc, char *argv[]) {
    int exit_code = main(argc, argv);
    exit(exit_code);
}
