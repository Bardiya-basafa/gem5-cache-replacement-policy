#include <stdint.h>
#include <stdio.h>
#define HOT_SIZE 128      // 256 ints = 1 KB (fits in L1)
#define ITERS    10000  // tuned for <45s in gem5

volatile int hot[HOT_SIZE];

int main() {
    for (int iter = 0; iter < ITERS; iter++) {
        for (int i = 0; i < HOT_SIZE; i++) {
            hot[i] += 1;
        }
    }
    printf("high locality finished\n");
    return 0;
}
