#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char** argv) {
    size_t KB = 1024, MB = 1024 * 1024;
    size_t hot_kb     = (argc > 1) ? strtoull(argv[1], NULL, 0) : 32;  // 32 KB
    size_t cold_mb    = (argc > 2) ? strtoull(argv[2], NULL, 0) : 4;   // 4 MB
    int    phases     = (argc > 3) ? atoi(argv[3]) : 2;                // 2 phases
    int    hot_reps   = (argc > 4) ? atoi(argv[4]) : 20;               // 20 loops
    size_t hot_stride = (argc > 5) ? strtoull(argv[5], NULL, 0) : 8;   // 8 elems

    size_t hot_n  = (hot_kb * KB) / sizeof(uint64_t);
    size_t cold_n = (cold_mb * MB) / sizeof(uint64_t);

    uint64_t *hot  = (uint64_t*) malloc(hot_n  * sizeof(uint64_t));
    uint64_t *cold = (uint64_t*) malloc(cold_n * sizeof(uint64_t));
    if (!hot || !cold) { perror("malloc"); return 1; }

    for (size_t i=0; i<hot_n;  i++) hot[i]  = i;
    for (size_t i=0; i<cold_n; i++) cold[i] = i;

    volatile uint64_t s = 0;
    for (int p=0; p<phases; p++) {
        for (int r=0; r<hot_reps; r++)
            for (size_t i=0; i<hot_n; i+=hot_stride) s += hot[i];
        for (size_t i=0; i<cold_n; i++) s += cold[i];
    }

    printf("sum=%llu\n", (unsigned long long)s);
    printf("phase change finished\n");
    free(hot); free(cold);
    return 0;
}