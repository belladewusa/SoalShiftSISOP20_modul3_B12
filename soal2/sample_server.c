// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include <sys/wait.h> 
#include <sys/types.h> 

#define PORT 8080 
#define MAX_CLIENT 30
#define FALSE 0
#define LOGIN 1
#define REGISTER 2
#define FIND_MATCH 3
#define LOGOUT 4

int checkInput(char *input) {
    int length = strlen(input), i;
    
    for ( i = 0; i < length; i++)
    {
        input[i] = tolower(input[i]);
    }
    
    if(!strcmp(input, "login")) return LOGIN;
    else if(!strcmp(input, "register")) return REGISTER;
    else if(!strcmp(input, "find match")) return FIND_MATCH;
    else if(!strcmp(input, "logout")) return LOGOUT;
    return FALSE;
}

void *client_thread( void *ptr );

int main(int argc, char const *argv[]) 
{ 
	int server_fd, new_socket
	[MAX_CLIENT]
	, valread, i, iret[MAX_CLIENT]; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address);  
	pthread_t threads[MAX_CLIENT];
	
	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 

	for ( i = 0; i < MAX_CLIENT; i++)
	{
		if ((new_socket[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}

		printf("New Client : %d\n", new_socket[i]);

		void *ress;
		int arr[1] = {new_socket[i]};

		iret[i] = pthread_create(&threads[i], NULL, client_thread, (void*) arr);
                
		if(iret[i]) //jika eror
		{
			fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i]);
			exit(EXIT_FAILURE);
		}
	}

	for ( i = 0; i < MAX_CLIENT; i++)
	{
		pthread_join(threads[i], NULL);
	}
	
	return 0;
} 

void *client_thread(void *ptr) {
	int valread;
	char buffer[1024], message[1024];
	int *new_socket = (int *)ptr;
	int client_addr = new_socket[0];
	memset(buffer, 0, sizeof(buffer));
	while ((valread = recv(client_addr , buffer , 1024 , 0)) > 0)
	{
		printf("%s\n", buffer);
		buffer[valread] = '\0'; 
		printf("%d %s\n", client_addr,buffer );
		sprintf(message, "%d", client_addr);
		write(client_addr , message , strlen(message));
		//clear the message buffer
		memset(message, 0, 1024);
		memset(buffer, 0, 1024);
	}
}