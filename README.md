# Implementaiton of Malloc and Free & Server Client application using POSIX shm

This repository contains implementation of
1. **malloc and free libraries using sbrk system call**

	- This can be found inside the folder /memory
	- memory.c contains the implementation of the library calls malloc_new() and free_new()
	- memory/main.c contains code to test the library calls 
	- memory folder also contains its own Makefile running which, user can build libraries for malloc_new() and free_new(), and as well as compile main.c

2. **Server Client application using POSIX shm**

	- User can build the application using `make` command

	- server.c contains code for implementation of server
	
    	- server reads data from client via shared shm and performs either one of the following operations:
			1. insert data into hashtable
			2. remove data from hashtable
			3. read data from 
         
         - running server
         		
               ./server -s 20
               
              1. command line parameter -s specifies the size of the hashtable that server holds during execution

	- client.c contains code for implementation of client

		- client sends data to server via shared shm. It can request to insert the data, read the data, or delete the data from hashtable.
		
		- running client
		
        	  ./client -o <operation> -k <key> -v <value>'
        
        	1. parameter -o accepts PUT, GET, DELETE values. It represents the operation type client wants server to perform
        	2. parameter -k passes the key 
        	3. parameter -v passes the value (required: when -o is PUT, optional: remaining time)
        	
		- Example:

			  ./client -o PUT -k 10 -v 20
              
          this command requests server to insert key value pair (10,20) into the hash table
          
        - **Note:** The client is parameter position sensitive, that is the command line arguments need to be in the order -o, -k and -v
