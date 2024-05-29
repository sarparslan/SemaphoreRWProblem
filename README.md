# SemaphoreRWProblem

## Overview
This C program demonstrates the implementation of the Reader-Writer problem using password authentication, threads, and semaphores. Each thread (reader or writer) authenticates using a password before performing read or write operations. The program employs POSIX threads (pthread) and Grand Central Dispatch (GCD) semaphores to manage concurrency.

## Features
- **Concurrency**: Uses pthreads to handle multiple reader and writer threads simultaneously.
- **Password Authentication**: Each thread must authenticate with a valid password before accessing the shared resource.
- **Resource Management**: Manages shared resource access using semaphores to ensure mutual exclusion and synchronization.

## Prerequisites
To run this program, you need a system with:
- GCC (GNU Compiler Collection)
- pthread support
- Access to POSIX compliant libraries

## Compilation
Compile the program using the following GCC command:

```bash
gcc main.c -o main_executable 
```

## Usage
Run the program by specifying the number of reader and writer threads:
```bash
./main_executable
```
The program will prompt you to enter the number of readers and writers:

- Number of readers - The number of reader threads to create
- Number of writers - The number of writer threads to create

## Implementation Details
- Thread Management: The program creates real and dummy reader and writer threads, each of which performs multiple operations.
- Password Generation: Generates random 6-digit passwords for thread authentication.
- Semaphore Usage: Uses GCD semaphores to manage access to the shared resource and ensure mutual exclusion.

## Example Output

<img width="539" alt="Screenshot 2024-05-29 at 21 53 04" src="https://github.com/sarparslan/SemaphoreRWProblem/assets/96438389/9e2eee3d-d9ac-4d4d-b166-eefad9d7ff97">








