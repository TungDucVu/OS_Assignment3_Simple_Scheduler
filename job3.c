#include <stdio.h>
#include <unistd.h>
#include "dummy_main.h"

int dummy_main(int argc, char **argv) {
    // Job that performs controlled output
    for (int i = 1; i <= 5; i++) {
        printf("Job 3 working... %d/6\n", i);
        sleep(1); // Simulates work for 1 second in each iteration
    }
    printf("Job 3 working... 6/6\n");
    return 0; // Exit after finishing
}
