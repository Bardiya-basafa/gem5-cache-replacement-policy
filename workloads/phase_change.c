#include <stdio.h>
#include <stdlib.h>

// --- TUNABLE PARAMETERS ---
// Size for the high-locality phase (Matrix Multiplication)
#define MATRIX_SIZE 64

// Size for the streaming phase (Stencil Calculation)
#define STENCIL_SIZE 16384 // 64KB array

// Number of timesteps to run the stencil in each streaming phase
#define STENCIL_TIMESTEPS_PER_PHASE 15

// Total number of phases to run (must be an even number to be balanced)
#define NUM_PHASES 6

// --- Phase 1: High Locality Workload ---
void run_locality_phase(int A[MATRIX_SIZE][MATRIX_SIZE],
                        int B[MATRIX_SIZE][MATRIX_SIZE],
                        int C[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            for (int k = 0; k < MATRIX_SIZE; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// --- Phase 2: Streaming Workload ---
void run_streaming_phase(int *current_grid, int *next_grid) {
    for (int t = 0; t < STENCIL_TIMESTEPS_PER_PHASE; ++t) {
        for (int i = 1; i < STENCIL_SIZE - 1; ++i) {
            next_grid[i] = (current_grid[i - 1] + current_grid[i] + current_grid[i + 1]) / 3;
        }
        // Swap pointers for the next iteration
        int *temp = current_grid;
        current_grid = next_grid;
        next_grid = temp;
    }
}

int main() {
    // --- Data for Locality Phase ---
    // Declared static to avoid stack overflow; puts them in .bss section.
    static int mat_A[MATRIX_SIZE][MATRIX_SIZE];
    static int mat_B[MATRIX_SIZE][MATRIX_SIZE];
    static int mat_C[MATRIX_SIZE][MATRIX_SIZE];

    // --- Data for Streaming Phase ---
    int *stencil_grid1 = (int*)malloc(STENCIL_SIZE * sizeof(int));
    int *stencil_grid2 = (int*)malloc(STENCIL_SIZE * sizeof(int));
    if (stencil_grid1 == NULL || stencil_grid2 == NULL) {
        printf("Error: Memory allocation failed!\n");
        return 1;
    }

    // --- Initialize all data structures ---
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            mat_A[i][j] = i; mat_B[i][j] = j; mat_C[i][j] = 0;
        }
    }
    for (int i = 0; i < STENCIL_SIZE; ++i) {
        stencil_grid1[i] = i % 100;
        stencil_grid2[i] = 0;
    }

    printf("Starting phase-change workload with %d phases...\n", NUM_PHASES);

    // --- Main Execution Loop ---
    for (int p = 0; p < NUM_PHASES; ++p) {
        if (p % 2 == 0) {
            // Even phases are high-locality
            printf("--- Starting Phase %d (Locality: Matrix Multiplication) ---\n", p);
            run_locality_phase(mat_A, mat_B, mat_C);
        } else {
            // Odd phases are streaming
            printf("--- Starting Phase %d (Streaming: Stencil Calculation) ---\n", p);
            run_streaming_phase(stencil_grid1, stencil_grid2);
        }
    }

    printf("Phase-change workload finished.\n");

    // Print a result from each workload to prevent optimization
    printf("Final Matrix Result C[0][0]: %d\n", mat_C[0][0]);
    printf("Final Stencil Result grid[SIZE/2]: %d\n", stencil_grid1[STENCIL_SIZE / 2]);

    free(stencil_grid1);
    free(stencil_grid2);

    return 0;
}