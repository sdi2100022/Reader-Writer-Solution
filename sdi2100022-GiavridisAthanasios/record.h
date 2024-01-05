#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* record structure */
typedef struct record{
    int client_id;
    char client_surname[20];
    char client_name[20];
    int client_balance;
}record;


/* Initializing an instance client of the structure type record 
 * with the given id,surname and name values, 
 * balance always starts at zero
 */
void Record_Init(record* client,int id,char* surname,char* name,int balance);

/* Print the values of the record client as follows :
 * [--ID--] [------SURNAME-----] [-------NAME-------] [---BALANCE---] 
 */
void print_Record(record client);