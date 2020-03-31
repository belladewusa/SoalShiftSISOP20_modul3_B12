#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> 
#define PORT 8600

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int opt = 1, addrlen = sizeof(address), max_sd, server_fd, new_socket, valread, client_socket[50] , max_clients = 50, sd, i, activity;
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    fd_set readfds; 
      
    for (i = 0; i < max_clients; i++) 
	{ 
		client_socket[i] = 0; 
	} 

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        FD_ZERO(&readfds); 

        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for ( i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if(sd > 0)
                FD_SET( sd , &readfds);

            if(sd > max_sd)
                max_sd = sd;
        }

        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);        
        
        if ((activity < 0) && (errno!=EINTR)) 
		{ 
			printf("select error"); 
		} 
    }
    
    // if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
    //     perror("accept");
    //     exit(EXIT_FAILURE);
    // }

    // do
    // {
    //     valread = read( new_socket , buffer, 1024);
    //     printf("%s\n",buffer );
    //     send(new_socket , hello , strlen(hello) , 0 );
    //     printf("Hello message sent\n");
    // } while (1);
    
    return 0;
}