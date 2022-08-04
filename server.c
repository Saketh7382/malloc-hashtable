#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "memory/memory.h"

#define SIZE 20
#define NAME "shared"

struct DataItem {
   int data;   
   int key;
};

typedef struct Data {
    int key;
    int val;
    struct Data *next;
} HashData;

HashData *HashTable[SIZE]; 

int hashCode(int key) {
   return key % SIZE;
}

struct Data *readdata(int key) {
    //get the hash 
    int hashIndex = hashCode(key);  

    if (HashTable[hashIndex] == NULL)
    {
        printf("No element found with key %d\n",key);
        return NULL;
    }
    else{
        HashData *current = HashTable[hashIndex]->next;
    
        while(1)
        {
            if (current->key == key)
            {
                printf("Found value associaled with key %d\n",key);
                return current;
            }
            else if (current->next == NULL)
            {
                printf("No element found with key %d\n",key);
                return NULL;
            }
            else{
                current = current->next;
            }
        }
    }  
}

void insertdata(int key,int data) {

    int index = hashCode(key);

    if (HashTable[index] == NULL)
    {
        
        printf("Creating new data node with key %d and data %d\n",key,data);

        HashData *datanode = (HashData*) malloc_new(sizeof(HashData));
        datanode->key = key;
        datanode->val = data;
        datanode->next = NULL;

        HashData *item = (HashData*) malloc_new(sizeof(HashData));
        item->next = datanode;

        HashTable[index] = item;
        
    }
    else{

        HashData *current = HashTable[index]->next;
        
        while(1)
        {
            if (current->key == key)
            {
                printf("Replacing data in key %d with new data %d\n",key,data);
                current->val = data;
                break;
            }
            else if (current->next == NULL)
            {
                printf("Inserting new data with key %d and data %d\n",key,data);
                HashData *datanode = (HashData*) malloc_new(sizeof(HashData));
                datanode->key = key;
                datanode->val = data;
                datanode->next = NULL;

                current->next = datanode;
                break;
            }
            else{
                current = current->next;
            }
        }
        
    }
    

}

struct Data *deletedata(int key) {

    int hashIndex = hashCode(key);

    if (HashTable[hashIndex] == NULL)
    {
        printf("Nothing to delete\n");
        return NULL;
    }
    else{
        HashData *prevnode = HashTable[hashIndex];
        HashData *datanode = HashTable[hashIndex]->next;
        HashData *nextnode = datanode->next;

        while(1)
        {
            if (datanode->key == key)
            {
                HashData *result = (HashData *)malloc_new(sizeof(HashData));
                result->key = datanode->key;
                result->val = datanode->val;
                printf("Got the value %d\n",result->val);

                prevnode->next = nextnode;

                if (nextnode == NULL)
                {
                    HashTable[hashIndex] = NULL;
                }
                free_new(datanode);
                return result;
            }
            else if (nextnode == NULL)
            {
                printf("Data with key %d not found in table\n",key);
                return NULL;
            }
            else{
                prevnode = datanode;
                datanode = nextnode;
                nextnode = nextnode->next;
            }
        }
    }      
}

void display() {
    int i = 0;
    
    for(i = 0; i<SIZE; i++) {
        if (HashTable[i] == NULL)
        {
            printf("%d --> NULL\n",i);
        }
        else{
            printf("%d --> ",i);
            HashData *current = HashTable[i]->next;
        
            while(1)
            {
                printf("%d --> ",current->val);
                if (current->next == NULL)
                {
                    printf("NULL\n");
                    break;
                }
                else{
                    current = current->next;
                }
            }
        }
    }
	
    printf("\n");
}

int main() {

    // printf("\n---------INSERT--------\n");

    // insertdata(1,10);
    // insertdata(2,10);
    // insertdata(42, 420);
    // insertdata(82, 820);
    // insertdata(3,30);
    // insertdata(3,60);

    // printf("\n---------READ--------\n");

    // HashData *retval = readdata(3);
    // if (retval != NULL)
    // {
    //     printf("The value is %d\n",retval->val);
    // }

    // retval = readdata(30);
    // if (retval != NULL)
    // {
    //     printf("The value is %d\n",retval->val);
    // }

    // printf("\n---------DELETE--------\n");


    // retval = deletedata(42);
    // if (retval != NULL)
    // {
    //     printf("Deleted the value %d\n",retval->val);
    // }

    // printf("\n---------READ--------\n");

    // retval = readdata(42);
    // if (retval != NULL)
    // {
    //     printf("The value is %d\n",retval->val);
    // }

    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("Error while creting / opening shared memory\n");
        exit(-1);
    }

    struct stat sb;
    int file_size;
    bool flag = true;

    while (1)
    {
        if (fstat(fd, &sb) == -1)
        {
            perror("Error while obtaining stats\n");
            exit(-2);
        }
        
        file_size = (int)sb.st_size;

        if (file_size > 0)
        {
            int *data = (int *)mmap(0, 3, PROT_READ, MAP_SHARED, fd, 0);
            int optype = data[0];

            if (optype == 1)
            {
                printf("INSERT - key: %d and value: %d\n",data[1],data[2]);
                insertdata(data[1],data[2]);

                ftruncate(fd, 2);
                int *reply = (int *)mmap(0, 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                reply[0] = 4; //ack
                reply[1] = 1;

                flag = true;
            }
            else if (optype == 2)
            {
                HashData *retval = readdata(data[1]);

                ftruncate(fd, 2);
                int *reply = (int *)mmap(0, 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                reply[0] = 4; //ack
                

                if (retval != NULL)
                {
                    printf("The value is %d\n",retval->val);
                    reply[1] = retval->val;
                }
                else{
                    reply[1] = -1; // could not found
                }

                flag = true;
            }
            else if (optype == 3)
            {
                HashData *retval = deletedata(data[1]);

                ftruncate(fd, 2);
                int *reply = (int *)mmap(0, 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                reply[0] = 4; //ack

                if (retval != NULL)
                {
                    printf("Deleted the value %d\n",retval->val);
                    reply[1] = retval->val;
                }
                else{
                    reply[1] = -1; //could not be found
                }

                flag = true;
            }
            else
            {
                if (flag)
                {
                    printf("Waiting for requests:\n");
                    flag = false;
                }
            }
            

            
        }
        

        sleep(1);
    }
}