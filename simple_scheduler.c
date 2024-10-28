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





typedef struct {

    char *command;

    char *name;

    pid_t pid;

    time_t start_time;

    time_t end_time;

    time_t wait_time;

    int priority;      

    int is_completed;

} Process;



Process ready_queue[MAX_READY_QUEUE];

int ready_queue_count = 0;

int num_cpus;

int timeslice;





Process completed_processes[MAX_READY_QUEUE];

int completed_count = 0;





void add_to_history(const char *command) {

    if (history_count < MAX_HISTORY) {

        history[history_count++] = strdup(command);

    }

}





void show_history() {

    for (int i = 0; i < history_count; i++) {

        printf("%d %s\n", i + 1, history[i]);

    }

}





void submit_process(const char *command) {

    if (ready_queue_count < MAX_READY_QUEUE) {



        int priority = 1;  

        char *cmd_copy = strdup(command);

        char *token = strtok(cmd_copy, " ");

        char *path = strdup(token);

        token = strtok(NULL, " ");  // Check for priority



        if (token != NULL) {

            priority = atoi(token);

            if (priority < 1 || priority > 4) priority = 1; // Ensure priority is in the range 1-4

        }

        free(cmd_copy);



        Process *new_process = &ready_queue[ready_queue_count++];

        new_process->command = strdup(path);

        new_process->name = strdup(path);

        new_process->pid = -1;

        new_process->start_time = 0;

        new_process->end_time = 0;

        new_process->wait_time = 0;

        new_process->priority = priority;

        new_process->is_completed = 0;



        printf("Process submitted: %s with priority %d\n", path, priority);

        free(path);

    } else {

        printf("Ready queue is full!\n");

    }

}





void execute_process(Process *process) {

    if (process->pid == -1) {



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



        kill(process->pid, SIGCONT);

    }



    sleep(timeslice);  // Simulate running the process for the timeslice

    kill(process->pid, SIGSTOP);  // Pause the process after the timeslice

}





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



        if (WIFEXITED(status) || WIFSIGNALED(status)) {

            process->is_completed = 1;

            process->end_time = time(NULL);

            return 1;

        }

    }

    return 0;

}





void round_robin_execution() {

    while (ready_queue_count > 0) {

        for (int priority = 1; priority <= 4; priority++) {

            for (int i = 0; i < ready_queue_count; i++) {

                Process *p = &ready_queue[i];

                

                if (p->priority == priority && !p->is_completed) {  // Process with current priority

                    for (int j = 0 ; j < ready_queue_count; j++) {

                        if (j != i && !ready_queue[j].is_completed) {

                            ready_queue[j].wait_time += timeslice;

                        }

                    }



                    execute_process(p);



                    // After executing, check if the process is completed

                    if (is_process_completed(p)) {

                        completed_processes[completed_count++] = *p;

                    } else {



                        Process temp = *p;

                        for (int j = i; j < ready_queue_count - 1; j++) {

                            ready_queue[j] = ready_queue[j + 1];

                        }

                        ready_queue[ready_queue_count - 1] = temp;

                        i--; 

                    }

                }

            }

        }





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





void print_process_results() {

    printf("\nProcess Results:\n");

    for (int i = 0; i < completed_count; i++) {

        Process *p = &completed_processes[i];

        printf("Process: %s (Priority: %d)\n", p->name, p->priority);

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





        line[nread - 1] = '\0';





        add_to_history(line);





        if (strcmp(line, "exit") == 0) {

            round_robin_execution();

            print_process_results();  // Print results after all processes finish

            break;

        }





        if (strcmp(line, "history") == 0) {

            show_history();

            continue;

        }





        if (strncmp(line, "submit ", 7) == 0) {

            submit_process(line + 7); // Add the command after "submit "

            continue;

        }

    }





    free(line);

    for (int i = 0; i < history_count; i++) {

        free(history[i]);

    }

    for (int i = 0; i < ready_queue_count; i++) {

        free(ready_queue[i].command);

    }

    return 0;

}

