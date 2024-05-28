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

// Function prototypes
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

    // Initializing semaphores
    read_mutex = dispatch_semaphore_create(1);  // read_mutex semaphore is created and assigned to 1
    write_mutex = dispatch_semaphore_create(1); // write_mutex semaphore is created and assigned to 1
    resource = dispatch_semaphore_create(1);    // resource semaphore is created and assigned to 1

    // Generating passwords
    generate_passwords();

    int num_readers, num_writers;

    // Geting the number of readers and writers from the user
    printf("Number of readers: ");
    scanf("%d", &num_readers);
    printf("Number of writers: ");
    scanf("%d", &num_writers);
    if((num_readers + num_writers > 10) ||num_readers < 1 || num_writers < 1){
        printf("There can be a minimum of 1 and a maximum of 9 readers and writers");
        return 0;
    }
    printf("\nThread No  Validity(real/dummy)  Role(reader/writer)  Value read/written\n");

    pthread_t readers[num_readers * 2]; //Multiplying by 2 for dummy readers
    pthread_t writers[num_writers * 2]; //Multiplying by 2 for dummy writers
    ThreadData reader_data[num_readers * 2]; // ThreadData struct for holding data for each reader thread
    ThreadData writer_data[num_writers * 2]; // ThreadData struct for holding data for each writer thread

    // Create real reader threads
    for (int i = 0; i < num_readers; i++) {
        reader_data[i].id = i + 1; // Assigning the id
        //strncpy function copies the password in passwords[i] to reader_data[i].password
        //PASSWORD_LENGTH + 1 refers to max number of character to copy
        strncpy(reader_data[i].password, passwords[i], PASSWORD_LENGTH + 1);
        strncpy(reader_data[i].type, "reader", 7); // Assigning the corresponding type
        reader_data[i].is_real = 1; // Assigning to 1 since it is a real thread
        pthread_create(&readers[i], NULL, reader, &reader_data[i]); // Creating a real reader thread and calling reader funtion
    }

    // Create dummy reader threads
    for (int i = 0; i < num_readers; i++) {
        reader_data[num_readers + i].id = num_readers + i + 1; // Assigning the id
        // Creating fake password
        snprintf(reader_data[num_readers + i].password, PASSWORD_LENGTH + 1, "%06d", rand() % 1000000);
        strncpy(reader_data[num_readers + i].type, "reader", 7); // Assigning the corresponding type
        reader_data[num_readers + i].is_real = 0; // Assigning to 0 since it is a dummy thread
        // Creating a dummy reader thread and calling reader funtion
        pthread_create(&readers[num_readers + i], NULL, reader, &reader_data[num_readers + i]);
    }

    // Create real writer threads
    for (int i = 0; i < num_writers; i++) {
        writer_data[i].id = i + 1; // Assigning the id
        //strncpy function copies the password in passwords[i] to writer_data[i].password
        //PASSWORD_LENGTH + 1 refers to max number of character to copy
        // passwords[num_readers + i] detects the  password to copy, num_readers + i is used to avoid mixing passwords
        strncpy(writer_data[i].password, passwords[num_readers + i], PASSWORD_LENGTH + 1);
        strncpy(writer_data[i].type, "writer", 7); // Assigning the corresponding type
        writer_data[i].is_real = 1; // Assigning to 1 since it is a real thread
        // Creating a real writer thread and calling writer funtion
        pthread_create(&writers[i], NULL, writer, &writer_data[i]);
    }

    // Create dummy writer threads
    for (int i = 0; i < num_writers; i++) {
        writer_data[num_writers + i].id = num_writers + i + 1; // Assigning the id
        // Creating fake password
        snprintf(writer_data[num_writers + i].password, PASSWORD_LENGTH + 1, "%06d", rand() % 1000000);
        strncpy(writer_data[num_writers + i].type, "writer", 7); // Assigning the corresponding type
        writer_data[num_writers + i].is_real = 0; // Assigning to 0 since it is a dummy thread
        // Creating a dummy writer thread and calling writer funtion
        pthread_create(&writers[num_writers + i], NULL, writer, &writer_data[num_writers + i]);
    }

    // Waiting all threads to complete
    for (int i = 0; i < num_readers * 2; i++) {
        pthread_join(readers[i], NULL);
    }

    for (int i = 0; i < num_writers * 2; i++) {
        pthread_join(writers[i], NULL);
    }

    // Releasing semaphores
    dispatch_release(read_mutex);
    dispatch_release(write_mutex);
    dispatch_release(resource);

    return 0;
}

void *reader(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        sleep(1);
        //Locking the read_mutex semaphore. If it is locked, waiting until it is released
        dispatch_semaphore_wait(read_mutex, DISPATCH_TIME_FOREVER);
        if (read_count == 0) { // Reader's first time
            //Locking the resource semaphore. If it is locked, waiting until it is released
            dispatch_semaphore_wait(resource, DISPATCH_TIME_FOREVER);
        }
        read_count++;
        dispatch_semaphore_signal(read_mutex); // Releasing the read_mutex semaphore

        if (is_valid_password(data->password)) { // If passwords matched
            //Printing ID, validity (real or dummy), role (reader), and value to read
            printf("%-10d %-20s %-20s %-20d\n",
                   data->id, data->is_real ? "real" : "dummy", data->type, BUFFER);
        } else {
            //Printing ID, validity (real or dummy), role (reader)
            //and  'No permission' since passwords are not the same
            printf("%-10d %-20s %-20s %-20s\n",
                   data->id, data->is_real ? "real" : "dummy", data->type, "No permission");
        }
        //LOCKİNG the read_mutex semaphore. If it is locked, waitİNG until it is released
        dispatch_semaphore_wait(read_mutex, DISPATCH_TIME_FOREVER);
        read_count--;
        if (read_count == 0) { // If this reader is the last reader
            dispatch_semaphore_signal(resource); //Releasing the resource semaphore
        }
        dispatch_semaphore_signal(read_mutex); // //Releasing the read_mutex semaphore
    }
    pthread_exit(0); // Terminating the thread
}

void *writer(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        sleep(1);
        //Locking the write_mutex semaphore. If it is locked, waiting until it is released
        dispatch_semaphore_wait(write_mutex, DISPATCH_TIME_FOREVER);
        
        //Locking the resource semaphore. If it is locked, waiting until it is released
        dispatch_semaphore_wait(resource, DISPATCH_TIME_FOREVER);

        if (is_valid_password(data->password)) { // If passwords matched
            //Assigning a random number between 0 and BUFFER_SIZE to the BUFFER variable
            BUFFER = rand() % BUFFER_SIZE;
            //Printing ID, validity (real or dummy), role (writer), and value to write
            printf("%-10d %-20s %-20s %-20d\n",
                   data->id, data->is_real ? "real" : "dummy", data->type, BUFFER);
        } else {
            //Printing ID, validity (real or dummy), role (writer)
            //and  'No permission' since passwords are not the same
            printf("%-10d %-20s %-20s %-20s\n",
                   data->id, data->is_real ? "real" : "dummy", data->type, "No permission");
        }

        dispatch_semaphore_signal(resource);    //Releasing the resource semaphore
        dispatch_semaphore_signal(write_mutex); //Releasing the write_mutex semaphore
    }
    pthread_exit(0); // Terminating the thread
}

void generate_passwords() {
    for (int i = 0; i < 10; i++) {
        //Generating a random number between 0 and 999999 and writing it as 6 digits to the passwords[i] array
        snprintf(passwords[i], PASSWORD_LENGTH + 1, "%06d", rand() % 1000000);
    }
}

int is_valid_password(const char *password) {
    for (int i = 0; i < 10; i++) {
        if (strncmp(password, passwords[i], PASSWORD_LENGTH) == 0) { //If the given password matches with passwords[i]
            return 1;
        }
    }
    return 0;
}

