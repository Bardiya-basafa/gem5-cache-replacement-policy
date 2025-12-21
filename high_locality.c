#include <stdio.h>

#define HOT_SIZE 64

int hot[HOT_SIZE];

int main() {
    for (int i = 0; i < 500000; i++) {
        hot[i % HOT_SIZE]++;
    }
    return 0;
}
