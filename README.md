# CISC 5595 Operating Systems Project (Round Robin Process Scheduler)

This program implements a round-robin scheduler in C. It demonstrates creating and managing child processes using the RR algorithm, where the user inputs a fixed quantum of time for processes to run before being paused and resuming another process.

This project was worked on by both me and [Jan Bierowiec](https://github.com/jbierowiec). 

## Features
1. **Process Creation**: Creates child processes and manages their execution lifecycle.
2. **RR Scheduling Algorithm**: Implements round-robin scheduling with configurable quantum times.
3. **Enhanced Metrics Reporting**:
   - **Response Time**: Time from process creation to its first execution.
   - **Turnaround Time**: Total time from process creation to completion.
4. **Verbose Mode**: An optional mode for detailed logs of process states and transitions.
5. **Dynamic Queue Implementation**: Uses a robust circular queue for managing processes.

## Project Structure

```plaintext
├── queue.c
├── queue.h
└── scheduler.c
```

## Enhancements

1. **Queue Enhancements**:
   - Improved error handling in `enqueue()` with specific messages when the queue is full.
   - Adjusted indexing logic in queue operations to prevent indexing issues.
2. **Signal Handling**:
   - Enhanced signal handlers for `SIGCONT` and `SIGSTOP` to toggle execution state.
   - Added detailed logging of signal actions for debugging.
3. **Verbose Mode**:
   - Optional third argument `[-v]` to provide detailed output for debugging and process monitoring.
4. **Timing Metrics**:
   - Uses monotonic clocks (`clock_gettime`) for accurate time calculations.
   - Prevents timing inaccuracies by initializing timing arrays to zero.

## How it Works

1. **Parent Process**: Creates a number of child processes using `fork()` and enqueues them.
2. **Child Processes**: Each child waits for a `SIGCONT` signal to start/resume execution.
3. **Execution Control**: Processes pause/resume based on the quantum, managed by the parent process.
4. **Metrics**: Records and displays average response and turnaround times for all processes.

## How to Compile and Run

1. Compile the code using `gcc`:
  ```bash
  gcc scheduler.c queue.c -o scheduler
  ```
2. Running the program:
  ```bash
  ./scheduler <number_of_processes> <quanta_ms> [verbose]
  ```
3. Example:
  ```bash
  ./scheduler 100 20
  ```
  or
   ```bash
  ./scheduler -v 100 20
  ```

## Additional Notes

1. Maximum number of processes: 1000 (configurable in the code).
2. Quantum time should be provided in milliseconds.
3. Verbose mode enables detailed process-level logs, such as process creation, signals received, and metrics.