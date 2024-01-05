#include "record.h"

void Record_Init(record* client,int id,char* surname,char* name,int balance){
    client->client_id = id;
    strcpy(client->client_surname,surname);
    strcpy(client->client_name,name);
    client->client_balance = balance;

    return;
}

void print_Record(record client){
    printf("%-6d %-20s %-20s %-16d\n",client.client_id,client.client_surname,client.client_name,client.client_balance);

    return;
}

