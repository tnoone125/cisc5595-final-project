#include <stdio.h>

int main(int argc, char **argv[]) {
    int created_pids[1000];
    bool amParent = false;

    // CREATE CHILD PROCESSES
    for (int i = 0; i < 1000; i++) {
        int rc = fork();
        if (rc == 0) { // ALL CHILD PROCESS CODE GOES HERE
            pause(); // WAIT FOR SIGNAL TO START!
            // Child Process, actually going to be a big loop
            for (int j = 0; j < 300000; j++) {
                printf("Hello Professor Dent from %s", getpid());
            }

            break; // Child process should not continue the loop and make its own children.
        } else if (rc > 0) { // this is parent path
            created_pids[i] = rc;
            amParent = true;
        }
    }


    // Send kill signals to our processes to start, stop, and resume them
    if (amParent) {
        while (queue not empty) {
            int pid = pull pid from queue
            kill(pid, SIGUSR1); // if process had already started, kill(pid, SIGCONT)
            sleep(quanta);
            kill(pid, SIGSTOP);
        }
    }
}