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
#define CONTEXT_SWITCH_OVERHEAD_MS 10


// this array stores process IDs
int created_pids[MAX_PROCESSES];

struct timespec first_response_times[MAX_PROCESSES];
struct timespec complete_times[MAX_PROCESSES];

// this is a flag that is controlled by signal handler
volatile sig_atomic_t run_process = 0;


// Signal handler to start/resume processes
void handle_signal(int signal) 
{
    run_process = 1;
}


// Function executed by each process
void execute_process(int pid_index) 
{
    long calculation = 0;


    for (int i = 1; i <= 1000; i++) 
    {
        if (!run_process) 
        {
            pause(); // Waits for the signal to resume
        }

        // calculation done by the processor
        // I'll send you what the outputs look like
        // Let me know if you want to change this to be a more complex calculation (Fibonacci, Brownian Motion, simple Physics Engine, etc)
        calculation += i * i;

        // instead of printing everything, I figured we can print out the progress of every 100 iterations 
        // When running the program, not everything would fit in a terminal, especailly for screenshots
        // I figured this part would be better simplified
        if (i % 100 == 0) 
        { 
            printf("Process %d: Calculation at step %d = %ld\n", getpid(), i, calculation);
        }
    }
    
    // added this to know when the process finished executing
    printf("Process %d: Finished execution.\n", getpid());
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <number_of_processes> <quanta_ms>\n", argv[0]);
        return 1;
    }


    int num_processes = atoi(argv[1]);
    int quanta = atoi(argv[2]);


    if (num_processes > MAX_PROCESSES || num_processes <= 0) {
        printf("Invalid number of processes. Max allowed is: %d\n", MAX_PROCESSES);
        return 1;
    }

    // when these arrays were integers, this caused the turnaround time to output 0
    // this helps output turnaround time wih realistic numbers
    memset(first_response_times, 0, sizeof(first_response_times));
    memset(complete_times, 0, sizeof(complete_times));


    signal(SIGUSR1, handle_signal); // signal handler

    // Process Queue initialization 
    Queue processQueue;
    initializeQueue(&processQueue);

    // CREATE CHILD PROCESSES
    for (int i = 0; i < num_processes; i++)
    {
        int rc = fork();
        if (rc == 0) 
        {            // ALL CHILD PROCESS CODE GOES HERE
            pause(); // WAIT FOR SIGNAL TO START!
            printf("Process %d started.\n", getpid());
            execute_process(i);
            exit(0); // Child process should not continue the loop and make its own children.
        }
        else if (rc > 0)
        { // this is parent path
            created_pids[i] = rc;
            // queue_times[i] = TODO
            enqueue(&processQueue, i);
        }
        else
        {
            perror("Fork failed");
            return 1;
        }
    }

    // Scheduler logic
    // removed the if statement 
    printf("Parent managing processes...\n");
    while (!isEmpty(&processQueue))
    {
        int i = poll(&processQueue);
        int pid = created_pids[i];

        /*
            Changed the response time code to get the current time, as well as I added the clock
            Earliler had clock(), but this did not output any time (all ouputs would be 0)
            So I changed it to CLOCK_MONOTONIC -> I provided the explanation for the difference between the two
        */

        // this is for getting the actual time (current time)
        struct timespec current_time;

        // clock() by itself gets CPU time but it isn't good for real-world time, so I added this instead
        // the monotonic clock gets the precise elapsed time
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        // helper if-statement to check if first response time is recorded 
        if (first_response_times[i].tv_sec == 0 && first_response_times[i].tv_nsec == 0) {
            first_response_times[i] = current_time; // Record first response time
        }

        kill(pid, SIGUSR1);    // Start or resume process
        // sleep() seconds
        // usleep() milliseconds
        usleep(quanta * 1000); // Allow process to run for quanta duration
        kill(pid, SIGSTOP);    // Preempt process

        // I added this to simulate the context switch overhead
        usleep(CONTEXT_SWITCH_OVERHEAD_MS * 1000);

        int status;            // Checks if the process is still running or completed
        int result = waitpid(pid, &status, WNOHANG);
        
        if (result == 0) 
        {
            // If the process is still running, then it is requeued it
            printf("Requeuing child process %d.\n", pid);
            enqueue(&processQueue, i);
        }

        // added the WIFEXITED to check if process has been completed
        // also records the completion time
        else if (WIFEXITED(status)) 
        {
            // complete times array
            clock_gettime(CLOCK_MONOTONIC, &complete_times[i]);
            printf("Child process %d completed.\n", pid);
        }
    }

    // This ensures that all the processes have finished
    for (int i = 0; i < num_processes; i++) {
        // printf("Letting all processes finish off...");
        // kill(created_pids[i], SIGCONT);
        // Wait for the child to exit
        waitpid(created_pids[i], NULL, 0);
    }


    // Added the for-loop below to calculate and print metrics
    // can technically be its own function (void) but I firgured I would add it here
    printf("\nProcess Metrics:\n");
    for (int i = 0; i < num_processes; i++) {
        
        long start_time_ms = first_response_times[i].tv_sec * 1000 + first_response_times[i].tv_nsec / 1000000;
        long end_time_ms = complete_times[i].tv_sec * 1000 + complete_times[i].tv_nsec / 1000000;
        long turnaround_time = end_time_ms - start_time_ms;

        printf("Process %d - Turnaround Time: %ld ms\n", created_pids[i], turnaround_time);
    }

    return 0;
}