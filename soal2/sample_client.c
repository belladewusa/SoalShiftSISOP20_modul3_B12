#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/types.h> 
#include <sys/wait.h> 


#define PORT 8080 
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

void clearscr ( void )
{
for ( int i = 0; i < 50; i++ ) // 50 is arbitrary
printf("\n");
}

int main(int argc, char const *argv[]) 
{ 
	int sock = 0, valread, is_login = 0; 
	struct sockaddr_in serv_addr; 
	char message[1024];
	char buffer[1024] = {0}; 

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        memset(message, 0, sizeof(message));
        refreshScreen();

        gets(message);
        puts(message);
        int input = checkInput(message);
        printf("%d\n", input);
        switch (input)
        {
        case LOGIN:
            send(sock , "Hai" , strlen("Hai") , 0 );
            send(sock , "Hello" , strlen("Hello") , 0 );
            send(sock , "Good" , strlen("Good") , 0 ); 
            break;
        case REGISTER:
            break;
        case FIND_MATCH:
            break;
        case LOGOUT:
            break;
        default:
            printf("Input Salah\n");
            break;
        }
        // send(sock , message , strlen(message) , 0 ); 
        // valread = read( sock , buffer, 1024); 
    }
    
	return 0; 
} 
