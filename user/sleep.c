#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <ticks>\n", argv[0]);
        exit(1);
    }
    char *arg = argv[1];
    int ticks = atoi(arg);
    sleep(ticks);
    exit(0);
}