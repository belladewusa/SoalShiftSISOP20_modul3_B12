// gcc -pthread -o soal4c soal4c.c
#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
#include <fcntl.h> 

#define WRITE 1
#define READ 0

int main() 
{ 
	int fd1[2]; 
    int status;

	char input_str[100]; 
	pid_t p;

	if (pipe(fd1) == -1) 
	{ 
		fprintf(stderr, "Pipe Failed" ); 
		return 1; 
	} 

	p = fork(); 

	if (p < 0) 
	{ 
		fprintf(stderr, "fork Failed" ); 
		return 1; 
	}

	if( p == 0 )
	{ 
		int status;

		pid_t child = fork();
		
        if(child == 0) {
			dup2(fd1[WRITE], STDOUT_FILENO);
			close(fd1[READ]);
			char *arr[] = {"ls", NULL};
			execvp("/bin/ls", arr);
		}
		while (wait(&status) > 0);
        
        close(fd1[WRITE]);
        dup2(fd1[READ], STDIN_FILENO);
		char *arr[] = {"wc","-l", NULL};
		execvp("/bin/wc", arr);
	}

	exit(EXIT_SUCCESS);
} 