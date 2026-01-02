#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memcpy

// --- TUNABLE PARAMETERS ---
// Array size: 16384 ints = 64KB. Two such arrays = 128KB.
// This is 4x the size of a 32KB L1D cache.
#define ARRAY_SIZE 16384

// Timesteps: Controls the runtime. The core loop runs ARRAY_SIZE * TIMESTEPS times.
// 30 timesteps makes the runtime comparable to the 64x64 matrix multiply.
#define TIMESTEPS 30

int main() {
    // Allocate two arrays on the heap: one for the current state, one for the next.
    int *current_grid = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *next_grid = (int*)malloc(ARRAY_SIZE * sizeof(int));

    if (current_grid == NULL || next_grid == NULL) {
        printf("Error: Memory allocation failed!\n");
        return 1;
    }

    // Initialize the grid with some starting values.
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        current_grid[i] = i % 100; // A simple pattern
        next_grid[i] = 0;
    }
    // Create a "hot spot" in the middle
    current_grid[ARRAY_SIZE / 2] = 1000;

    printf("Starting %d timesteps of 1D stencil calculation...\n", TIMESTEPS);

    // --- The Core Stencil Workload ---
    for (int t = 0; t < TIMESTEPS; ++t) {
        // This inner loop streams through `current_grid` to compute `next_grid`.
        // This is the classic streaming access pattern that challenges LRU.
        for (int i = 1; i < ARRAY_SIZE - 1; ++i) {
            // Each point is the average of its neighbors from the previous step.
            next_grid[i] = (current_grid[i - 1] + current_grid[i] + current_grid[i + 1]) / 3;
        }

        // Swap the pointers for the next iteration. This is much faster than copying.
        // `next_grid` becomes the `current_grid` for the next timestep.
        int *temp = current_grid;
        current_grid = next_grid;
        next_grid = temp;
    }

    printf("Stencil calculation finished.\n");

    // Print a result to prevent the compiler from optimizing the work away.
    printf("Result grid[ARRAY_SIZE/2]: %d\n", current_grid[ARRAY_SIZE / 2]);

    // Clean up the allocated memory.
    free(current_grid);
    free(next_grid);

    return 0;
}