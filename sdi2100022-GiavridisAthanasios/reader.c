#include "memory.h"

#define OPT_STRING "f:l:d:s:"
#define FLAG_FILENAME 'f'
#define FLAG_RECID 'l'
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
    if (argc!=9){    
      	printf("Incorrect Syntax\n"); //error prompt
        printf("Correct Syntax is: %s -%c filename -%c recid[,recid] -%c time -%c shmid\n",argv[0],FLAG_FILENAME,FLAG_RECID,FLAG_TIME,FLAG_SHMID); //correct syntax example
      	
        return -1; //error code
   	}


    /* Variable Section */
    int opt_arg; //storing the value of the getopt function
    bool check_filename = false,check_recid = false,check_time = false,check_shmid = false; //flags for arguments to check if all arguments have been satisfied

    FILE *fpb; //file pointer for the binary input file
        char filename[20]; //the name of the file
        int num_accounts; //number of accounts inside the file
        int blocksize; //number of accounts inside a critical section

    int recid; //the account id of the first recid
        int recid_opt = 0; //the account id of the optional recid,setting to minus one cause not always available

    int sleep_secs; //the time the reader process will "sleep" in second(s)

    int shmid = -1; //shared memory object id
    void* ptr; //pointer to the memory of the shared memory object
    shmem* memory; //= malloc(sizeof(shmem)); //pointer to the structure of the shared memory object

    float avg_balance = 0;//calculating average balance
    int count = 0; //count for average balance

    int first_block; //first block id of the critical section
    int last_block; //last block id of the critical section
    
    /* Getting terminal arguments using getopt */
    while((opt_arg = getopt(argc,argv,OPT_STRING)) != -1){
        switch (opt_arg){ //checking all flags for 
            case FLAG_FILENAME:

                strcpy(filename,optarg); //copy the filename

                fpb = fopen(optarg,"rb"); //opening binaryfile with read policy
                if(fpb == NULL){
                    printf("Cannot Open Binary File\n"); //error prompt
                    
                    return -1; //error code
                }
                
                fseek (fpb , 0 , SEEK_END);
                int lsize = ftell (fpb);
                rewind (fpb);
                num_accounts = (int) lsize/sizeof(record); //calculating total accounts in the binary file
                blocksize = num_accounts * 1/BLOCKS; //calculating the size of each block
                
                check_filename = true; //set filename flag as true cause it is satisfied
                break;

            case FLAG_RECID:

                char* token = strtok(optarg, ","); //token to split the argument between the comma
                if(token == NULL){
                    printf("Invalid Format: %s\n",optarg); //error prompt
                    printf("Correct Format: recid,[recid]\n"); //correct format

                    return -1; //error code
                }

                recid = atoi(token);
                token = strtok(NULL,",");

                if(token != NULL){ //if there was another recid after a comma
                    recid_opt = atoi(token); //set recid_opt as the second id

                    if(recid_opt < recid){ //if optional id is larger switching so that recid_opt always has the bigger value
                        int temp;
                        temp = recid;
                        recid = recid_opt;
                        recid_opt = temp;
                    }

                }

                check_recid = true; //set recid flag as true cause it is satisfied
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
                if(shmid == -1){//checking if it exists
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

    if(recid <= 0 || recid > num_accounts){
        printf("There is no client with account id: %d\n",recid); //error prompt

        return -1; //error code
    }

    if(recid_opt > num_accounts){ //if user gave a number larger than the max accounts use max accounts instead
        recid_opt = num_accounts; //set recid_opt as the last available id
    }

    if(recid_opt == 0){ //if user didnt give a second recid
        recid_opt = recid; //treat it as the same value;
    }

    first_block = recid / blocksize ; //calculating the first block id of the critical section
    last_block = recid_opt / blocksize; //calculating the last block id of the critical section
    
    if(first_block == BLOCKS){ //if its the last record of the file 
        first_block--; //decrement to not go over the block limit
    }

    if(last_block == BLOCKS){ //if its the last record of the file 
        last_block--; //decrement to not go over the block limit
    }

    /* Start of Critical Section */

    for(int i = first_block; i <= last_block;i++){
        if(i == first_block){ //we calculate only the first wait time 
            wait_str = (double)times(&tb_wait_str); //start counting wait time
        }

        /* Semaphores */
        int value_sem;
        sem_getvalue(&memory->block[i].qmutex,&value_sem);
        if(value_sem == 0){ //if it will wait
            printf("\n....waiting... \n\n"); //wait prompt
        }

        sem_wait(&memory->block[i].qmutex);
        if(memory->block[i].active_readers == 0){   
            sem_getvalue(&memory->block[i].mutex,&value_sem);
            if(value_sem == 0){ //if it will wait
                printf("\n..waiting... \n\n"); //wait prompt
            }
            sem_wait(&memory->block[i].mutex); //if the block is blocked,wait till it unblocks 

        }
        sem_post(&memory->block[i].qmutex); //let the next process proceed
        memory->block[i].active_readers++;
        
        if(i == first_block){ //we calculate only the first wait time 
            wait_end = (double)times(&tb_wait_end); //stop counting wait time
            wait_time = (wait_end - wait_str)/ticspersec; //calculate wait time 
        }   
        /* Reading clients' accounts */
        
        /* Reader Process Sleeps */
        sleep(sleep_secs);

        int id = recid; //initialize the starting id

        while(id <= recid_opt && id <= blocksize*(i+1)){ //if we have read all the requested clients or the id is not in this block currently in the critical section
            record client;
            fseek(fpb, sizeof(record)*(id - 1), SEEK_SET); //seeking the record client inside the file
            fread(&client,sizeof(record),1,fpb); //reading it
            print_Record(client);
            id++; //incrementing the counter
            count++; //incremented the number of accounts to calculate average balance
            avg_balance += client.client_balance; //adding to the total balance the balance of the current processed client
        }

        recid = id; //store the last id you read

        /* Semaphores */
            
        
        memory->block[i].active_readers--; //decrement the active readers
        if(memory->block[i].active_readers == 0){ //if there are not active readers unblock a writer
            sem_post(&memory->block[i].mutex);
        }
    }

    /* End Of Critical Section */

    /* Average Balance */
    avg_balance = avg_balance/count; //calculating average
    printf("\nAverage Client Balance: %.2f\n",avg_balance);

    total_end = (double)times(&tb_total_end); //stop counting total time
    total_time = (total_end - total_str)/ticspersec; //calculate total time

    /* Update Statistics */
    memory->num_readers += 1; //incrementing the readers' counter
    memory->num_recs_processed += count; //incrementing the number of records processed
    
    if(memory->max_wait_time < wait_time){ //calculating max wait time between all processes
        memory->max_wait_time = wait_time; //set the new max
        strcpy(memory->process,"a Reader"); //by a Reader
    }

    memory->total_reader_time += total_time; //adding to the total time spent by readers

    /* Closing Documents */
    close(shmid); //close memory object
    fclose(fpb); //close binary file
    return 0;
}