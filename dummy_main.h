#ifndef DUMMY_MAIN_H
#define DUMMY_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// Prototype for user-defined main
int dummy_main(int argc, char **argv);

int main(int argc, char **argv) {
    // Add any necessary initialization for the scheduler
    int ret = dummy_main(argc, argv);
    return ret;
}
#define main dummy_main

#endif // DUMMY_MAIN_H
