#include "memory.h"

int main(int argc,char* argv[])
{

    int shmid; //shared memory object id
    void* ptr; //pointer to the memory of the shared memory object
    shmem* memory; //pointer to the structure of the shared memory object
    double avg = 0.0; //calculating the averages

    /* Opening Shared Memory Object*/
    shmid = shm_open(SHM_OBJECT,O_RDWR,0666); //opening the shared memory object 
    if(shmid == -1){ //checking if it exists
        printf("Shared Memory Object Does Not Exist\n"); //error prompt

        return -1; //error code
    }

    ptr = mmap(0,BYTE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmid,0);
    memory = ptr;

    /* Printing Statistics */
    printf("Total Number of Readers worked on the file were %d\n",memory->num_readers);
    if(memory->num_readers != 0){ //checking if dividing by zero
        avg = memory->total_reader_time / memory->num_readers;
    }
    printf("Average Time for Readers was %lf\n",avg);
    avg = 0;
    printf("Total Number of Writers worked on the file were %d\n",memory->num_writers);
    if(memory->num_writers != 0){ //checking if dividing by zero
        avg = memory->total_writer_time / memory->num_writers;
    }
    printf("Average Time for Writers was %lf\n",avg);

    printf("Longest Response Time of a process was %lf by %s\n",memory->max_wait_time,memory->process);
    printf("Total Number of Records Processed was %d\n",memory->num_recs_processed);

    
    /* Closing Documents */

    for(int i = 0;i < BLOCKS;i++){ //destroying the semaphores of each block
        sem_destroy(&memory->block[i].mutex);
        sem_destroy(&memory->block[i].qmutex);
    }

    close(shmid); //close memory object   
    shm_unlink(SHM_OBJECT); //unlink the shared memory object

    return 0;
}