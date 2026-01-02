#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char** argv) {
    size_t MB = 1024 * 1024;
    size_t bytes = (argc > 1) ? strtoull(argv[1], NULL, 0) : (4ULL * MB); // 4 MB
    size_t n = bytes / sizeof(uint64_t);

    uint64_t *a = (uint64_t*) malloc(n * sizeof(uint64_t));
    if (!a) { perror("malloc"); return 1; }
    for (size_t i = 0; i < n; i++) a[i] = i;

    volatile uint64_t s = 0;
    for (size_t i = 0; i < n; i++) s += a[i];

    printf("sum=%llu\n", (unsigned long long)s);
    printf("streaming finished\n");
    free(a);
    return 0;
}