#include <stdio.h>
#include <unistd.h>
#include "dummy_main.h"

int dummy_main(int argc, char **argv) {
    // Job that simulates work by sleeping
    for (int i = 1; i <= 2; i++) {
        printf("Job 2 working... %d/3\n", i);
        sleep(1); // Simulates work for 1 second in each iteration
    }
    printf("Job 2 working... 3/3\n");
    return 0; // Exit after completing work
}
