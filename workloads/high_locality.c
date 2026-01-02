#include <stdio.h>
#include <stdlib.h>

// --- TUNABLE PARAMETER ---
// This size is chosen to be slightly larger than a typical 32KB L1D cache.
// 3 matrices * 64 * 64 * sizeof(int) = 48KB, which creates cache pressure.
#define MATRIX_SIZE 64

// A simple matrix multiplication function to generate memory accesses
// with high spatial and temporal locality.
// Note the C-style syntax for passing a 2D array.
void matrix_multiply(int A[MATRIX_SIZE][MATRIX_SIZE],
                     int B[MATRIX_SIZE][MATRIX_SIZE],
                     int C[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            for (int k = 0; k < MATRIX_SIZE; ++k) {
                // This line is the core of the memory access pattern.
                // It repeatedly accesses a row of A and a column of B.
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    // In C, we declare fixed-size 2D arrays on the stack.
    // For this small size, this is efficient and simple.
    int A[MATRIX_SIZE][MATRIX_SIZE];
    int B[MATRIX_SIZE][MATRIX_SIZE];
    int C[MATRIX_SIZE][MATRIX_SIZE];

    // Initialize matrices to ensure the data is touched.
    // Must explicitly initialize C to zero.
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            A[i][j] = i;
            B[i][j] = j;
            C[i][j] = 0;
        }
    }

    // This is the main part of the workload that will be simulated.
    // It's short, but generates a lot of localized memory traffic.
    printf("Starting %dx%d matrix multiplication...\n", MATRIX_SIZE, MATRIX_SIZE);

    matrix_multiply(A, B, C);

    printf("Multiplication finished.\n");

    // Print a result to prevent the compiler from optimizing the work away.
    printf("Result C[0][0]: %d\n", C[0][0]);

    return 0; // Or EXIT_SUCCESS
}