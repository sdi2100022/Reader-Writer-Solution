# Κ22 - Operating Systems Project 3
## Reader/Writer Problem

### Author: Αθανάσιος Γιαβρίδης  
### ΑΜ: 1115-2021-00022    
### email: sdi2100022@di.uoa.gr

## Σχεδιαστικές Επιλογές
 - Χρήση ανεξάρτητου προγράμματος για δημιουργία του  ***shared memory segment***.
 - Το ***shared memory segment*** είναι διασπαμένο, σε διάφορες μεταβλητές η κάθε μια με χρήση που φαίνεται δίπλα από την δήλωση τους, χρησιμοποιώντας την εξής δομή ***shmem***:

		typedef  struct  shm_segments{
			char  filename[20]; //for error catching,no user give different binary file name to the same shared memory object
			int  num_readers; //total number of readers who worked on the file
			double  total_reader_time; //total summation of all time worked by readers
			int  num_writers; //total number of writers who worked on the file
			double  total_writer_time; //total summation of all time worked by writers
			int  num_recs_processed; //tolal number of records processed by both readers/writers
			double  max_wait_time; //the most time a process(reader or writer) took start
			char  process[12]; //name of process of max wait time
			
			sem_cs  block[BLOCKS]; //array of blocks with their own semaphores
		}shmem;
- Το ***sem_cs*** είναι και αυτό με την σειρά του μία δομή η οποία έχει τους σεμαφόρους του κάθε critical section.

		/* Semaphores for each block of the binary file*/
		typedef  struct  sem_cs{
			sem_t  mutex; //semaphore for blocked critical section
			sem_t  qmutex; //semaphore for readers/writers that are waiting,like a queue
			int  active_readers; //number of active readers on the block
		}sem_cs;
 - Οι δύο σεμαφόροι  χρησιμοποιούνται για : ο **mutex** υπάρχει για να μπλοκάρει τους writers από τους readers και τους readers,writers όταν γράφει ένας writer και ο **qmutex** λειτουγεί σαν ουρά ώστε να επικρατεί η πολιτική **FIFO** στις διεργασίες.
 - Ο αλγόριθμος είναι δηλαδή :
####  Readers:
		/* Start Of Critical Section */
		
		Finds the critical section that it is gonna read from
		Checks if the current critical section is blocked or not by a waiting process
			sem_wait(&qmutex);
		Checks if its the first reader on this critical section
			if(active_readers == 0)
				Blocks Writers
					sem_wait(&mutex);
		Frees the next waiting process in queue
			sem_post(&qmutex);
		Increments active readers
			active_readers++;

		/* Reads From File */

		Decrements active readers
			active_readers--;
		Checks if it was the last reader on this critical section
			if(active_readers == 0)
				Unblocks Writers
					sem_post(&mutex);
						
		/* End Of Critical Section */
#### Writers :
		/* Start Of Critical Section */
		
		Finds the critical section that it is gonna write to
		Checks if the current critical section is blocked or not by a waiting process
			sem_wait(&qmutex);
		Blocks Readers/Writers
			sem_wait(&mutex);
		Frees the next waiting process in queue
			sem_post(&qmutex);
			
		/* Writes To File */

		Unblocks Readers/Writers
			sem_post(&mutex);
			
		/* End Of Critical Section */
- Το αρχείο έχει χωριστεί σε διαφορετικά Blocks για να υπάρχουν διαφορετικά  critical sections. To πλήθος έχει αρχικοποιηθεί σε 10 αλλά μπορεί να μεταβληθεί στο πηγαίο αρχείο ***memory.h***.
 

		 /* each block will span (1/BLOCKS)% of the total records,
		  * if you alter the number try to have whole number records in each block
		  * /
		  #define  BLOCKS  10 

- Κάθε σετ από σεμαφόρους ανήκει στο δικό του Block του αρχείου.  
-  Το ***-s shmid*** δεν είναι id αλλά το όνομα του ***shared memory segment*** διότι χρησιμοποιείται η συνάρτηση ***shm_open();*** για την πρόσβαση στην κοινή μνήμη των διεργασιών. Το όνομα έχει αρχικοποιηθεί σε *shared_memory* αλλά μπορεί να μεταβληθεί στο πηγαίο αρχείο ***memory.h***.

			#define SHM_OBJECT  "shared_memory"
- Η εκτέλεση των αναγνωστών/συγγραφεών γίνεται από διαφορετικά **ttys**.
 - Χρήση ανεξάρτητου προγράμματος για καταστροφή του ***shared memory segment***.
 


## Οδηγίες Χρήσης Εφαρμογής
### makefile
Με την εντολή **make** δημιουργούνται τα εκτελέσιμα προγράμματα ***./linker***, ***./reader***, ***./writer***, ***./unlinker***.
> make 

Επίσης με την εντολή **make clean** διαγράφονται όλα τα εκτελέσιμα καθώς και τα αντικείμενα προγράμματα.
> make clean

### Program Calls
Στην αρχή πρέπει να καλέσουμε το ***./linker*** ώστε να δημιουργηθεί το shared memory object της εφαρμογής.

> ./linker
#### Output : 
		The shmid name is: [--SHM_OBJECT--]
Έπειτα μπορούμε να καλέσουμε τα main προγράμματά μας ***./reader***, ***./writer*** με τον εξής τρόπο.
Κλήση του αναγνώστη:
> ./reader -f filename -l recid[,recid] -d time -s shmid
#### Output:
		[--ID--] [------SURNAME-----] [-------NAME-------] [---BALANCE---]
		[--ID--] [------SURNAME-----] [-------NAME-------] [---BALANCE---]
		[--ID--] [------SURNAME-----] [-------NAME-------] [---BALANCE---]
		....
		[--ID--] [------SURNAME-----] [-------NAME-------] [---BALANCE---]
		
		Average Client Balance : [-AVG-BALANCE-]

Κλήση του συγγραφέα:
> ./writer -f filename -l recid -d time -v value -s shmid
#### Output:
		[--ID--] [------SURNAME-----] [-------NAME-------] [-NEW-BALANCE-]

Τα ορίσματα των προγραμμάτων μπορούν να δωθούν με όποια σειρά θελήσετε.
Αν κάποιο πρόγραμμα περιμένει κάποιο άλλο πρόγραμμα τότε θα εμφανιστεί στην οθόνη :
#### Output:
		...waiting....
Στο όρισμα ***-s shmid*** είναι απαραίτητο να δωθεί το όνομα που εκτυπώθηκε από την εκτέλεση του ***./linker***.

Για να τερματίσουμε την εφαρμογή και να εκτυπώσουμε τα στατιστικά καλούμε το ***./unlinker***.
> ./unlinker
#### Output: 
		Total Number of Readers worked on the file were [--READERS--]
		Average Time for Readers was [-AVG-READ-TIME-]
		Total Number of Writers worked on the file were [--WRITERS--]
		Average Time for Writers was [-AVG-WRITE-TIME-]
		Longest Response Time of a process was [-MAX--RESPONSE-] by [-PROCESS--TYPE-]
		Total Number of Records Processed was [-TOTAL--RECORDS-]
