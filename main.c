#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <dispatch/dispatch.h>

#define PASSWORD_LENGTH 6
#define NUM_OPERATIONS 5
#define BUFFER_SIZE 10000

// Global variables
int BUFFER;
char passwords[10][PASSWORD_LENGTH + 1];
dispatch_semaphore_t read_mutex, write_mutex, resource;
int read_count = 0;

// Function declarations
void *reader(void *arg);
void *writer(void *arg);
void generate_passwords();
int is_valid_password(const char *password);

typedef struct {
    int id;
    char password[PASSWORD_LENGTH + 1];
    char type[7]; // reader or writer
    int is_real; // 1 for real, 0 for dummy
} ThreadData;

int main() {
    srand(time(NULL)); // To make random numbers different from each other

    // Initialize semaphores
    read_mutex = dispatch_semaphore_create(1);
    write_mutex = dispatch_semaphore_create(1);
    resource = dispatch_semaphore_create(1);

    // Generate passwords
    generate_passwords();

    int num_readers, num_writers;

    // Get the number of readers and writers from the user
    printf("Number of readers: ");
    scanf("%d", &num_readers);
    printf("Number of writers: ");
    scanf("%d", &num_writers);
    if(num_readers > 9 || num_writers > 9 || num_readers < 1 || num_writers < 1){
        printf("There can be a minimum of 1 and a maximum of 9 readers and writers");
    }
    printf("\nThread No  Validity(real/dummy)  Role(reader/writer)  Value read/written\n");

    pthread_t readers[num_readers * 2];
    pthread_t writers[num_writers * 2];
    ThreadData reader_data[num_readers * 2];
    ThreadData writer_data[num_writers * 2];

    // Create real reader threads
    for (int i = 0; i < num_readers; i++) {
        reader_data[i].id = i + 1;
        strncpy(reader_data[i].password, passwords[i], PASSWORD_LENGTH + 1);
        strncpy(reader_data[i].type, "reader", 7);
        reader_data[i].is_real = 1;
        pthread_create(&readers[i], NULL, reader, &reader_data[i]);
    }

    // Create dummy reader threads
    for (int i = 0; i < num_readers; i++) {
        reader_data[num_readers + i].id = num_readers + i + 1;
        snprintf(reader_data[num_readers + i].password, PASSWORD_LENGTH + 1, "%06d", rand() % 1000000);
        strncpy(reader_data[num_readers + i].type, "reader", 7);
        reader_data[num_readers + i].is_real = 0;
        pthread_create(&readers[num_readers + i], NULL, reader, &reader_data[num_readers + i]);
    }

    // Create real writer threads
    for (int i = 0; i < num_writers; i++) {
        writer_data[i].id = i + 1;
        strncpy(writer_data[i].password, passwords[num_readers + i], PASSWORD_LENGTH + 1);
        strncpy(writer_data[i].type, "writer", 7);
        writer_data[i].is_real = 1;
        pthread_create(&writers[i], NULL, writer, &writer_data[i]);
    }

    // Create dummy writer threads
    for (int i = 0; i < num_writers; i++) {
        writer_data[num_writers + i].id = num_writers + i + 1;
        snprintf(writer_data[num_writers + i].password, PASSWORD_LENGTH + 1, "%06d", rand() % 1000000);
        strncpy(writer_data[num_writers + i].type, "writer", 7);
        writer_data[num_writers + i].is_real = 0;
        pthread_create(&writers[num_writers + i], NULL, writer, &writer_data[num_writers + i]);
    }

    // Join all threads
    for (int i = 0; i < num_readers * 2; i++) {
        pthread_join(readers[i], NULL);
    }

    for (int i = 0; i < num_writers * 2; i++) {
        pthread_join(writers[i], NULL);
    }

    // Release semaphores
    dispatch_release(read_mutex);
    dispatch_release(write_mutex);
    dispatch_release(resource);

    return 0;
}

void *reader(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        sleep(1);
        dispatch_semaphore_wait(read_mutex, DISPATCH_TIME_FOREVER);
        if (read_count == 0) {
            dispatch_semaphore_wait(resource, DISPATCH_TIME_FOREVER);
        }
        read_count++;
        dispatch_semaphore_signal(read_mutex);

        if (is_valid_password(data->password)) {
            printf("%-10d %-20s %-20s %-20d\n",
                   data->id, data->is_real ? "real" : "dummy", data->type, BUFFER);
        } else {
            printf("%-10d %-20s %-20s %-20s\n",
                   data->id, data->is_real ? "real" : "dummy", data->type, "No permission");
        }

        dispatch_semaphore_wait(read_mutex, DISPATCH_TIME_FOREVER);
        read_count--;
        if (read_count == 0) {
            dispatch_semaphore_signal(resource);
        }
        dispatch_semaphore_signal(read_mutex);
    }
    pthread_exit(0);
}

void *writer(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        sleep(1);
        dispatch_semaphore_wait(write_mutex, DISPATCH_TIME_FOREVER);
        dispatch_semaphore_wait(resource, DISPATCH_TIME_FOREVER);

        if (is_valid_password(data->password)) {
            BUFFER = rand() % BUFFER_SIZE;
            printf("%-10d %-20s %-20s %-20d\n",
                   data->id, data->is_real ? "real" : "dummy", data->type, BUFFER);
        } else {
            printf("%-10d %-20s %-20s %-20s\n",
                   data->id, data->is_real ? "real" : "dummy", data->type, "No permission");
        }

        dispatch_semaphore_signal(resource);
        dispatch_semaphore_signal(write_mutex);
    }
    pthread_exit(0);
}

void generate_passwords() {
    for (int i = 0; i < 10; i++) {
        snprintf(passwords[i], PASSWORD_LENGTH + 1, "%06d", rand() % 1000000);
    }
}

int is_valid_password(const char *password) {
    for (int i = 0; i < 10; i++) {
        if (strncmp(password, passwords[i], PASSWORD_LENGTH) == 0) {
            return 1;
        }
    }
    return 0;
}
