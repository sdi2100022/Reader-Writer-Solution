//#include <stdio.h> already in record.h
//#include <stdlib.h> already in record.h
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
//#include <string.h> already in record.h
#include <time.h>

#include <fcntl.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/mman.h> //shm_open and shm_unlink
#include <sys/stat.h>
#include <sys/times.h> //calculating time

#include "record.h"

#define BYTE_SIZE 4096
#define SHM_OBJECT "shared_memory"

/* each block will span (1/BLOCKS)% of the total records,
 * if you alter the number try to have whole number records in each block
 */
#define BLOCKS 10 

/* Semaphores for each block of the binary file*/
typedef struct sem_cs
{
    sem_t mutex; //semaphore for blocked critical section
    sem_t qmutex; //semaphore for readers/writers that are waiting,like a queue
    int active_readers; //number of active readers on the block
}sem_cs;

/* Struct of the Shared Memory Object*/
typedef struct shm_segments
{
    char filename[20]; //for error catching,no user give different binary file name to the same shared memory object

    int num_readers; //total number of readers who worked on the file
    double total_reader_time; //total summation of all time worked by readers
    int num_writers; //total number of writers who worked on the file
    double total_writer_time; //total summation of all time worked by writers
    int num_recs_processed; //tolal number of records processed by both readers/writers
    double max_wait_time; //the most time a process(reader or writer) took start
    char process[12]; //name of process of max wait time
    
    sem_cs block[BLOCKS]; //array of blocks with their own semaphores

}shmem;

#define SIZE_SHMEM sizeof(shmem); //size of struct