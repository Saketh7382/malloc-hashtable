#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define NAME "shared"

void client();

int main(int argc, char *argv[]){

    char* op = argv[2];
    int optype;

    if (strcmp(op,"PUT") == 0)
    {
        optype = 1;
    }
    else if (strcmp(op,"GET") == 0)
    {
        optype = 2;
    }
    else if (strcmp(op,"DELETE") == 0)
    {
        optype = 3;
    }
    else{
        printf("Invalid argument %s\n",op);
        exit(0);
    }
    
    int key = atoi(argv[4]);
    int val;

    if (optype == 1)
    {
        val = atoi(argv[6]);
    }
    else{
        val = -1;
    }
    
    
    client(optype,key,val);

    return 0;
}

void client(int optype, int key, int val){
    
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("Error while creting / opening shared memory\n");
        exit(-1);
    }

    int size;

    /*
    1 -> insert
    2 -> read
    3 -> delete
    4 -> ack
    */

    if (optype == 1)
    {
        size = 2;
    }
    else{
        size = 1;
    }
    
    
    ftruncate(fd, size+1);

    int *data = (int *)mmap(0, 2+1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    data[0] = optype;

    if (optype == 1)
    {
        data[1] = key;
        data[2] = val;
    }
    else{
        data[1] = key;
    }

    munmap(data, size+1);

    while (1)
    {
        int *reply = (int *)mmap(0, 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (reply[0] == 4)
        {
            if (optype == 1)
            {
                printf("Insertion successfull\n");
            }
            else if (optype == 2)
            {
                if (reply[1] == -1)
                {
                    printf("No data with key: %d\n",key);
                }
                else{
                    printf("Got data: %d\n",reply[1]);
                }
                
            }
            else if (optype == 3)
            {
                if (reply[1] == -1)
                {
                    printf("No data with key: %d\n",key);
                }
                else{
                    printf("Successfully deleted key: %d with value: %d\n",key,reply[1]);
                }
            }
            
            
            break;
        }
        sleep(1);
    }
    

    close(fd);
}