#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

#define MAX_HISTORY 100
#define MAX_COMMAND_LENGTH 1024
#define MAX_READY_QUEUE 100

// Global history array
char *history[MAX_HISTORY];
int history_count = 0;

// Struct to store process information
typedef struct {
    char *command;
    char *name;
    pid_t pid;
    time_t start_time;
    time_t end_time;
    time_t wait_time;
    int is_completed;
} Process;

Process ready_queue[MAX_READY_QUEUE];
int ready_queue_count = 0;
int num_cpus;
int timeslice;

// Additional array for completed processes
Process completed_processes[MAX_READY_QUEUE];
int completed_count = 0;

// Function to add command to history
void add_to_history(const char *command) {
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(command);
    }
}

// Function to display history
void show_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
}

// Function to add a process to the ready queue
void submit_process(const char *command) {
    if (ready_queue_count < MAX_READY_QUEUE) {
        Process *new_process = &ready_queue[ready_queue_count++];
        new_process->command = strdup(command);
        new_process->name = strdup(command);
        new_process->pid = -1;
        new_process->start_time = 0;
        new_process->end_time = 0;
        new_process->wait_time = 0;
        new_process->is_completed = 0;
        printf("Process submitted: %s\n", command);
    } else {
        printf("Ready queue is full!\n");
    }
}

// Function to execute or resume a process in the ready queue
void execute_process(Process *process) {
    if (process->pid == -1) {
        // First time execution (fork and exec)
        char *args[MAX_COMMAND_LENGTH / 2 + 1];
        int i = 0;
        char *token = strtok(process->command, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        pid_t pid = fork();
        if (pid == 0) {  // Child process
            if (execvp(args[0], args) == -1) {
                perror("SimpleShell");
            }
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            process->pid = pid;
            process->start_time = time(NULL);
        } else {
            perror("SimpleShell: Fork failed");
        }
    } else {
        // Resume the process from where it left off
        kill(process->pid, SIGCONT);
    }

    sleep(timeslice);  // Simulate running the process for the timeslice
    kill(process->pid, SIGSTOP);  // Pause the process after the timeslice
}

// Function to check if a process is completed
int is_process_completed(Process *process) {
    int status;
    pid_t result = waitpid(process->pid, &status, WNOHANG);  // Check non-blocking
    if (result == 0) {
        // Process still running
        return 0;
    } else if (result == -1) {
        perror("Error during waitpid");
        return 1;  // Consider process completed on error
    } else {
        // Process finished
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            process->is_completed = 1;
            process->end_time = time(NULL);
            return 1;
        }
    }
    return 0;
}



// Updated Function to simulate round-robin execution
void round_robin_execution() {
    while (ready_queue_count > 0) {
        for (int i = 0; i < ready_queue_count; i++) {
            Process *p = &ready_queue[i];
            for (int j = 0 ; j < ready_queue_count; j++) {
                if (j != i && !ready_queue[j].is_completed) {
                    ready_queue[j].wait_time += timeslice;
                }
            }
            

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
        printf("Process: %s\n", p->name);
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
