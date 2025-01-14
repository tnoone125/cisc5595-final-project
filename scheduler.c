// Jan C. Bierowiec
// Thomas Noone
// Operating Systems Final Project
// Round Robin Scheduling Algorithm

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "queue.h"

// maximum processes and a context switch overhead
#define MAX_PROCESSES 1000

// this array stores process IDs and allows us to "key in" for stats
int created_pids[MAX_PROCESSES];

// Print more details about queuing of processes and their progress.
bool verbose = false;

// these arrays keep track of generation, response & complete times.
// The indexes correspond to the index of created_pids for
// accessing the actual PID.
struct timespec process_gen_times[MAX_PROCESSES];
struct timespec first_response_times[MAX_PROCESSES];
struct timespec complete_times[MAX_PROCESSES];

// this is a flag that is controlled by signal handler
volatile sig_atomic_t run_process = 0;

// Signal handler to start/resume processes
void handle_sigcont(int signal)
{
    run_process = 1;
    if (verbose)
        printf("PID %d received a start signal.\n", getpid());
}

void handle_sigstop(int signal)
{
    run_process = 0;
    if (verbose)
        printf("PID %d received a stop signal.\n", getpid());
}

// Function executed by each process
void execute_process()
{
    srand48(getpid() ^ time(NULL));
    int runs = (int)(drand48() * (50000000 - 10000 + 1)) + 10000; // Random integer between 10000 and 50000000
    printf("Process %d started: it will sum i^2 up to i = %d.\n", getpid(), runs);

    long calculation = 0L;
    for (unsigned int i = 0; i < runs; i++)
    {
        if (!run_process)
        {
            raise(SIGSTOP); // Waits for the signal to resume
        }

        calculation += i * i;

        if (i % 500000 == 0 && verbose)
        {
            printf("Process %d is currently at step %d.\n", getpid(), i);
        }
    }

    printf("Process %d completed. Answer: %ld\n", getpid(), calculation);
}


int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4) {
        //printf("Usage: %s <number_of_processes> <quanta_ms>\n", argv[0]);
        printf("Usage: %s <number_of_processes> <quanta_ms> [verbose]\n", argv[0]);
        return 1;
    }

    int add_arg = 0;

    if (argc == 4) {
        add_arg = 1;
        if (strcmp(argv[1], "-v") == 0) {
            verbose = true;
        }
    }

    int num_processes = atoi(argv[1+add_arg]);
    int quanta = atoi(argv[2+add_arg]);

    if (num_processes > MAX_PROCESSES || num_processes <= 0) {
        printf("Invalid number of processes. Max allowed is: %d\n", MAX_PROCESSES);
        return 1;
    }

    if (verbose) {
        printf("Verbose mode enabled.\n");
    }

    // when these arrays were integers, this caused the turnaround time to output 0
    // this helps output turnaround time wih realistic numbers
    memset(first_response_times, 0, sizeof(first_response_times));
    memset(complete_times, 0, sizeof(complete_times));

    // Process Queue initialization
    Queue processQueue;
    initializeQueue(&processQueue);

    // CREATE CHILD PROCESSES
    for (int i = 0; i < num_processes; i++)
    {
        int rc = fork();
        if (rc == 0)
        {            // ALL CHILD PROCESS CODE GOES HERE
            //printf("Process %d created!\n", getpid());
            if (verbose) {
                printf("Process %d created!\n", getpid());
            }
            signal(SIGCONT, handle_sigcont);
            signal(SIGSTOP, handle_sigstop);
            raise(SIGSTOP); // WAIT FOR SIGNAL TO START!
            execute_process();
            exit(0); // Child process should not continue the loop and make its own children.
        }
        else if (rc > 0)
        { // this is parent path
            struct timespec current_time;
            clock_gettime(CLOCK_MONOTONIC, &current_time);

            created_pids[i] = rc;
            process_gen_times[i] = current_time;
            enqueue(&processQueue, i);
        }
        else
        {
            perror("Fork failed");
            return 1;
        }
    }

    //printf("Parent managing processes...\n");
    if (verbose) {
        printf("Parent managing processes...\n");
    }
    while (!isEmpty(&processQueue))
    {
        int i = poll(&processQueue);
        int pid = created_pids[i];

        // this is for getting the actual time (current time)
        struct timespec current_time;

        // clock() by itself gets CPU time but it isn't good for real-world time, so I added this instead
        // the monotonic clock gets the precise elapsed time
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        // helper if-statement to check if first response time is recorded
        if (first_response_times[i].tv_sec == 0 && first_response_times[i].tv_nsec == 0) {
            first_response_times[i] = current_time; // Record first response time
        }

        kill(pid, SIGCONT);    // Start or resume process
        // sleep() seconds
        // usleep() microseconds
        usleep(quanta * 1000); // Allow process to run for quanta duration

        int status;            // Checks if the process is still running or completed
        int result = waitpid(pid, &status, WNOHANG);

        if (result == -1)
        {
            perror("Error fetching status of pid.\n");
        }
        else if (result == 0)
        {
            kill(pid, SIGSTOP);
            enqueue(&processQueue, i);
            if (verbose) {
                printf("Child process %d is still running, so it has been stopped and returned to the queue.\n", pid);
                printf("Child process %d is still running, so it has been stopped and returned to the queue.\n", pid);
            }
        }
        else if (WIFEXITED(status))
        {
            clock_gettime(CLOCK_MONOTONIC, &complete_times[i]);
            if (verbose) {
                printf("Child process %d completed, not requeueing.\n", pid);
            }
        }
        else if (WIFSIGNALED(status))
        {
            clock_gettime(CLOCK_MONOTONIC, &complete_times[i]);
            if (verbose) {
                printf("Child process %d terminated with signal: %d. Not requeuing.\n", pid, WTERMSIG(status));
            }
        }
    }

    printf("\nProcess Metrics:\n");
    long sum_response_times = 0L;
    long sum_turnaround_times = 0L;
    for (int i = 0; i < num_processes; i++) {
        long gen_time_ms = process_gen_times[i].tv_sec * 1000 + process_gen_times[i].tv_nsec / 1000000;
        long start_time_ms = first_response_times[i].tv_sec * 1000 + first_response_times[i].tv_nsec / 1000000;
        long end_time_ms = complete_times[i].tv_sec * 1000 + complete_times[i].tv_nsec / 1000000;
        long turnaround_time = end_time_ms - gen_time_ms;
        long response_time = start_time_ms - gen_time_ms;

        sum_response_times += response_time;
        sum_turnaround_times += turnaround_time;
        if (verbose) {
            printf("Process %d - Response Time: %ld ms\n", created_pids[i], response_time);
            printf("Process %d - Turnaround Time: %ld ms\n", created_pids[i], turnaround_time);
        }
    }

    printf("Average response time: %.2f ms\n", (double)sum_response_times / num_processes);
    printf("Average turnaround time: %.2f ms\n", (double)sum_turnaround_times / num_processes);

    return 0;
}
