#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#define PORT 8080


  
int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread, flag = -1, i, status, is_login = 0;
    struct sockaddr_in serv_addr;
    char input[100];
    char buffer[1024] = {0};
    pid_t child;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    
    do
    {   
        child = fork();
        if(child == 0) {
            char *arr[] = {'clear', NULL};
            execv("/bin/clear", arr);
        }
        while (wait(&status));
        
        if(flag == 0)
            printf("Insert The Correct Command");
        
        flag = -1;

        gets(input);

        int length = strlen(input);
        
        for ( i = 0; i < length; i++)
        {
            input[i] = tolower(input[i]);
        }
        
        if(strcmp(input, "login") == 0) {
            send(sock , hello , strlen(hello) , 0 );
            is_login = 1;
        }else if(strcmp(input, "find") == 0 && is_login){

        }else if(strcmp(input, "logout" == 0 && is_login)){
            is_login = 0;
        }else{
            flag = 0;
        }
        // printf("Hello message sent\n");
        // valread = read( sock , buffer, 1024);
        // printf("%s\n",buffer );
    } while (1);
    
    return 0;
}