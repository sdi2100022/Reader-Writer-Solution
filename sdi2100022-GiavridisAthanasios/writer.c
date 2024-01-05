#include "memory.h"

#define OPT_STRING "f:l:v:d:s:"
#define FLAG_FILENAME 'f'
#define FLAG_RECID 'l'
#define FLAG_VALUE 'v'
#define FLAG_TIME 'd'
#define FLAG_SHMID 's'

int main(int argc,char* argv[]){
    /* For time calculation */
    double total_str,total_end,wait_str,wait_end;
    struct tms tb_total_str,tb_total_end,tb_wait_str,tb_wait_end;
    double ticspersec = (double)sysconf(_SC_CLK_TCK);
    double total_time,wait_time;

    total_str = (double)times(&tb_total_str); //start counting total time

    /* Checking for incorrect syntax */
    if (argc!=11){    
      	printf("Incorrect Syntax\n"); //error prompt
        printf("Correct Syntax is: %s -%c filename -%c recid -%c value -%c time -%c shmid\n",argv[0],FLAG_FILENAME,FLAG_RECID,FLAG_VALUE,FLAG_TIME,FLAG_SHMID); //correct syntax example
      	
        return -1; //error code
   	}

    /* Variable Section */
    int opt_arg; //storing the value of the getopt function
    bool check_filename = false,check_recid = false,check_value = false,check_time = false,check_shmid = false; //flags for arguments to check if all arguments have been satisfied

    FILE *fpb; //file pointer for the binary input file
        char filename[20]; //the name of the file
        int num_accounts; //number of accounts inside the file
        int blocksize; //number of accounts inside a block

    int recid; //the account id of recid
    
    int value; //value shift of the client's balance

    int sleep_secs; //the time the writer process will "sleep" in second(s)

    int shmid = -1; //shared memory segment id
    void* ptr; //pointer to the memory of the shared memory object
    shmem* memory; //pointer to the structure of the shared memory object

    int block_id; //block id of the critical section


    
    /* Getting terminal arguments using getopt */
    while((opt_arg = getopt(argc,argv,OPT_STRING)) != -1){
        switch (opt_arg){ //checking all flags for 
            case FLAG_FILENAME:

                strcpy(filename,optarg); //copy the filename                
            
                fpb = fopen(optarg,"r+b"); //opening binaryfile with read write policy
                if(fpb == NULL){
                    printf("Cannot Open Binary File\n"); //error prompt
                    
                    return -1; //error code
                }
                
                fseek (fpb , 0 , SEEK_END);
                int lsize = ftell (fpb);
                rewind (fpb);
                num_accounts = (int) lsize/sizeof(record); //calculating total accounts in the binary file
                blocksize = num_accounts/BLOCKS; //calculating the size of each block

                check_filename = true; //set filename flag as true cause it is satisfied
                break;

            case FLAG_RECID:

                recid = atoi(optarg);
                if(recid <= 0){ //if user gave zero or negative number
                    recid = 1; //set it to the first available account id
                }

                check_recid = true; //set recid flag as true cause it is satisfied
                break;

            case FLAG_VALUE:

                value = atoi(optarg);

                check_value = true; //set value flag as true cause it is satisfied
                break;

            case FLAG_TIME:

                srand(time(NULL));
                int max_time = atoi(optarg);
                if(max_time < 1){ //if user gives zero or negative value 
                    max_time = 1; //set max_time to one second
                }
                sleep_secs = (rand() % max_time) + 1; //set sleep_secs from 1 up to max_time second(s)

                check_time = true;  //set time flag as true cause it is satisfied
                break;

            case FLAG_SHMID:
 
                if(strcmp(optarg,SHM_OBJECT) != 0){ //checking if its the correct name
                    printf("Incorrect Shared Memory Object Name\n"); //error prompt

                    return -1; //error code
                }
 
                shmid = shm_open(optarg,O_RDWR,0666); //opening the shared memory object 
                if(shmid == -1){ //checking if it exists
                    printf("Shared Memory Object Does Not Exist\n"); //error prompt

                    return -1; //error code
                }

                ptr = mmap(0,BYTE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmid,0);
                memory = ptr;

                check_shmid = true; //set shmid flag as true cause it is satisfied
                break;
        }
    }

    /* Checking if the arguments' flags were satisfied */

    if(check_filename == false){
        printf("Invalid Format: -%c argument is missing\n",FLAG_FILENAME); //error prompt

        return -1; //error code
    }

    if(check_recid == false){
        printf("Invalid Format: -%c argument is missing\n",FLAG_RECID); //error prompt

        return -1; //error code
    }

    if(check_value == false){
        printf("Invalid Format: -%c argument is missing\n",FLAG_VALUE); //error prompt

        return -1; //error code
    }
    
    if(check_time == false){
        printf("Invalid Format: -%c argument is missing\n",FLAG_TIME); //error prompt

        return -1; //error code
    }

    if(check_shmid == false){
        printf("Invalid Format: -%c argument is missing\n",FLAG_SHMID); //error prompt

        return -1; //error code
    }

    if(strcmp(memory->filename,"Nofile") == 0){ //first time opening the file
        strcpy(memory->filename,filename); //store the intended filename to the shared memory object
    }

    if(strcmp(memory->filename,filename) != 0){ //if filename differs from indended filename throw an error
        printf("Incorrect Binary File\n"); //error prompt
        printf("The Binary File is : %s\n",memory->filename); //correct filename

        return -1; //error code
    }

    if(recid > num_accounts){
        printf("There is no client with account id: %d\n",recid); //error prompt

        return -1; //error code
    }

    block_id = recid / blocksize; //calculating the block id of the critical section

    /* Start of Critical Section */

    wait_str = (double)times(&tb_wait_str); //stop counting wait time
    
    int value_sem;
    sem_getvalue(&memory->block[block_id].qmutex,&value_sem);
    if(value_sem == 0){ //if it will wait
        printf("\n...waiting.. \n\n"); //wait prompt
    }
    sem_wait(&memory->block[block_id].qmutex); //wait in "queue" and block all other process after

    sem_getvalue(&memory->block[block_id].mutex,&value_sem);
    if(value_sem == 0){ //if it will wait
        printf("\n...waiting.... \n\n"); //wait prompt
    }
    sem_wait(&memory->block[block_id].mutex); //wait if another process blocks and block all other process after cause a writer is writing
    sem_post(&memory->block[block_id].qmutex); //move the "queue" to the next process
    
    wait_end = (double)times(&tb_wait_end); //stop counting wait time
    wait_time = (wait_end - wait_str)/ticspersec; //calculate wait time 

    /* Writer Process Sleeps */
    sleep(sleep_secs);

    /* Writing client's account */
    record client;
    fseek(fpb, sizeof(record)*(recid - 1), SEEK_SET); //seeking the record client inside the file
    fread(&client,sizeof(record),1,fpb); //reading it to change its balance
        
    client.client_balance += value; //shifting the balance of the client by value ammount
    fseek(fpb, sizeof(record)*(recid - 1), SEEK_SET); //seeking the record client inside the file
    fwrite(&client,1,sizeof(record),fpb); //writing the new balance
    print_Record(client); //printing the client's account

    sem_post(&memory->block[block_id].mutex); //unblock the section as there is no writer on it anymore

    /* End Of Critical Section */

    total_end = (double)times(&tb_total_end); //stop counting total time
    total_time = (total_end - total_str)/ticspersec; //calculate total time

    /* Update Statistics */
    memory->num_writers += 1; //incrementing the writers' counter
    memory->num_recs_processed += 1; //incrementing the number of records processed

    if(memory->max_wait_time < wait_time){ //calculating max wait time between all processes
        memory->max_wait_time = wait_time; //set the new max
        strcpy(memory->process,"a Writer"); //by a writer
    }


    memory->total_writer_time += total_time; //adding to the total time spent by writers
    
    /* Closing Documents */
    close(shmid); //close memory object
    fclose(fpb); //close binary file

    return 0;
}