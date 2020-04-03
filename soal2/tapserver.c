// gcc -pthread -o tapserver tapserver.c 
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
#define TRUE 1
#define LOGIN 2
#define REGISTER 3
#define FIND_MATCH 4
#define LOGOUT 5
#define WIN 6
#define LOSE 7

int searching;
int health[100];

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
    FILE *fptr;
    fptr = fopen("akun.txt","ab");
    fclose(fptr);

    searching = -1;
    for ( i = 0; i < 100; i++)
        health[i] = 100;
	
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
	char buffer[1024], message[1024], username[1024], password[1024];
	int *new_socket = (int *)ptr;
	int client_addr = new_socket[0];
    FILE *fptr;
	
    memset(buffer, 0, sizeof(buffer));
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(message, 0, sizeof(message));

	while ((valread = recv(client_addr , buffer , 1024 , 0)) > 0)
	{
        if(!strcmp("login", buffer)) {
            fptr = fopen("akun.txt","rb");
            int flag = FALSE;
            if(fptr != NULL) {
                memset(buffer, FALSE, sizeof(buffer));

                valread = recv(client_addr , username , 1024 , 0);
                valread = recv(client_addr , password , 1024 , 0);

                char temp[2][1024];

                while(fread(temp, sizeof(temp), 2, fptr))
                {
                    if(!strcmp(temp[0], username) && !strcmp(temp[1], password)) {
                        flag = TRUE;
                        send(client_addr , "login success" , strlen("login success") , 0 );
                        printf("Auth success\n");
                        break;
                    }
                }

                fclose(fptr);
            }

            if(!flag){
                send(client_addr , "login failed" , strlen("login failed") , 0 );
                printf("Auth Failed\n");
            }

        }else if(!strcmp("register", buffer)) {
            fptr = fopen("akun.txt","ab");
            char temp[2][1024];
            
            memset(buffer, FALSE, sizeof(buffer));
            
            valread = recv(client_addr , temp[0] , 1024 , 0);
            valread = recv(client_addr , temp[1] , 1024 , 0);

            send(client_addr , "register success" , strlen("register success") , 0 );

            fwrite(temp , sizeof(temp) , 2 ,  fptr );

            fclose(fptr);

            fptr = fopen("akun.txt","rb");
            if(fptr != NULL) {
                while (fread(temp, sizeof(temp), 2, fptr))
                {
                    printf("Username : %s Password : %s\n", temp[0], temp[1]);
                }
                
                fclose(fptr);
            }
        }else if(!strcmp("waiting for player", buffer)) {
            int opponent_addr;
            if(searching == -1){
                searching = client_addr;

                while (searching == client_addr)
                {
                }

                opponent_addr = searching;

            } else {
                opponent_addr = searching;
                searching = client_addr;

                sleep(1);
                searching = -1;
                send(client_addr, "found your opponent", strlen("found your opponent"), 0);
                send(opponent_addr, "found your opponent", strlen("found your opponent"), 0);
                health[client_addr] = health[opponent_addr] = 100;
            }

            int flag = WIN;

            while ((valread = recv(client_addr , buffer , 1024 , 0)) > 0)
            {
                health[opponent_addr] -= 10;

                if(health[opponent_addr] <= 0) {
                    send(client_addr, "win", strlen("win"), 0);
                    send(opponent_addr, "lose", strlen("lose"), 0);
                    break;
                }

                if(health[client_addr] <= 0) {
                    send(client_addr, "lose", strlen("lose"), 0);
                    send(opponent_addr, "win", strlen("win"), 0);
                    break;
                }

                send(client_addr, "hit !!", strlen("hit !!"), 0);
                
                memset(message, FALSE, sizeof(message));
                sprintf(message, "Health kamu sekarang : %d", health[opponent_addr]);
                send(opponent_addr, message, sizeof(message), 0);
                memset(buffer, 0, sizeof(buffer));
            }
            memset(buffer, 0, sizeof(buffer));
        }
		
        memset(buffer, 0, sizeof(buffer));
        memset(username, 0, sizeof(username));
        memset(password, 0, sizeof(password));
        memset(message, 0, sizeof(message));
	}
}