#include "memory.h"

int main(int argc, char* argv[]){
    /* Variable Section */
    int shmid; //shared memory object id
    void* ptr; //pointer to the memory of the shared memory object
    shmem* memory; //pointer to the structure of the shared memory object

    /* Share Memory Object Initialization */
    shmid = shm_open(SHM_OBJECT,O_CREAT|O_RDWR,0666); //open the shared memory object and get its id
    if(shmid == -1){ //check if shm_open worked
        printf("Shared Memory Object named: %s already exists\n",SHM_OBJECT); //error prompt

        return -1;//error code
    }

    if(ftruncate(shmid,BYTE_SIZE) == -1){ //configure the size of the shared memory object 
        return -1;//error code
    }

    /* Memory Mapping */   
    ptr = mmap(0,BYTE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmid,0); //memory map the shared memory object

    memory = ptr; //casting the structure shmem on the shared memory object

    /* initializing the values of the structure */
    strcpy(memory->filename,"Nofile");

    memory->num_readers = 0;
    memory->total_reader_time = 0;
    memory->num_writers = 0;
    memory->total_writer_time = 0;
    memory->num_recs_processed = 0;
    memory->max_wait_time = 0;
    strcpy(memory->process,"No Process");
    for(int i = 0;i < BLOCKS;i++){
        sem_init(&memory->block[i].mutex,1,1);
        sem_init(&memory->block[i].qmutex,1,1);
        memory->block[i].active_readers = 0;
    }



    /* Print the shmid to the user */
    printf("The shmid name is: %s\n",SHM_OBJECT);

    close(shmid); //close memory object

    return 0;
}