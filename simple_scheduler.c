// Additional array for completed processes
Process completed_processes[MAX_READY_QUEUE];
int completed_count = 0;

// Updated Function to simulate round-robin execution
void round_robin_execution() {
    while (ready_queue_count > 0) {
        for (int i = 0; i < ready_queue_count; i++) {
            Process *p = &ready_queue[i];

            // Only execute if the process has not completed
            if (!p->is_completed) {
                execute_process(p);

                // After executing, check if the process is completed
                if (is_process_completed(p)) {
                    // Move completed process to the completed_processes array
                    completed_processes[completed_count++] = *p;
                } else {
                    // Move the current process to the back of the queue
                    Process temp = *p;
                    for (int j = i; j < ready_queue_count - 1; j++) {
                        ready_queue[j] = ready_queue[j + 1];
                    }
                    ready_queue[ready_queue_count - 1] = temp;
                    i--;  // Adjust for the changed order
                }
            }
        }

        // Remove finished processes from the ready queue
        int i = 0;
        while (i < ready_queue_count) {
            if (ready_queue[i].is_completed) {
                free(ready_queue[i].command);
                for (int j = i; j < ready_queue_count - 1; j++) {
                    ready_queue[j] = ready_queue[j + 1];
                }
                ready_queue_count--;
            } else {
                i++;
            }
        }
    }
}

// Updated function to print process results after all processes finish
void print_process_results() {
    printf("\nProcess Results:\n");
    for (int i = 0; i < completed_count; i++) {
        Process *p = &completed_processes[i];
        printf("Process: %s\n", p->command);
        printf("Time Taken: %f seconds\n", difftime(p->end_time, p->start_time));
        printf("Waiting Time: %ld seconds\n\n", p->wait_time);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_cpus> <timeslice>\n", argv[0]);
        return EXIT_FAILURE;
    }

    num_cpus = atoi(argv[1]);
    timeslice = atoi(argv[2]);

    printf("Number of CPUs: %d\n", num_cpus);
    printf("Timeslice: %d seconds\n", timeslice);

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (1) {
        printf("simple-shell> ");
        nread = getline(&line, &len, stdin);
        if (nread == -1) {
            perror("getline");
            exit(EXIT_FAILURE);
        }

        // Remove newline character
        line[nread - 1] = '\0';

        // Add command to history
        add_to_history(line);

        // Check if command is "exit"
        if (strcmp(line, "exit") == 0) {
            round_robin_execution();
            print_process_results();  // Print results after all processes finish
            break;
        }

        // Check if command is "history"
        if (strcmp(line, "history") == 0) {
            show_history();
            continue;
        }

        // Check if command is "submit"
        if (strncmp(line, "submit ", 7) == 0) {
            submit_process(line + 7); // Add the command after "submit "
            continue;
        }
    }

    // Cleanup and exit
    free(line);
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }
    for (int i = 0; i < ready_queue_count; i++) {
        free(ready_queue[i].command);
    }
    return 0;
}
