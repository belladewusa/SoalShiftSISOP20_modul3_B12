// gcc tapplayer.c -pthread -o tapplayer
// do sudo apt-get install libncurses5-dev
// then "sudo apt install ncurses-doc" to see man ncurses for doc
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/types.h> 
#include <sys/wait.h>
// #include <curses.h>
#include <pthread.h>
#include <stdlib.h>
#include <termios.h>

#define PORT 8080 
#define FALSE 0
#define TRUE 1
#define LOGIN 2
#define REGISTER 3
#define FIND_MATCH 4
#define LOGOUT 5
#define UNAUTHENTICATED 6
#define AUTHENTICATED 7
#define BATTLE 8
#define WIN 9
#define LOSE 10

int status, sock, valread;

static struct termios stored_settings;

void set_keypress(void)
{
    struct termios new_settings;
    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0,TCSANOW,&new_settings);
    return;
}

void reset_keypress(void)
{
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}

int checkInput(char *input) {
    int length = strlen(input), i;
    
    for ( i = 0; i < length; i++)
    {
        input[i] = tolower(input[i]);
    }
    
    if(!strcmp(input, "login")) return LOGIN;
    else if(!strcmp(input, "register")) return REGISTER;
    else if(!strcmp(input, "find match") || !strcmp(input, "find")) return FIND_MATCH;
    else if(!strcmp(input, "logout")) return LOGOUT;
    return FALSE;
}

void clearscr ( void )
{
    int i;
    for ( i = 0; i < 50; i++ )
        putchar('\n');
}

void *loop( void *ptr )
{
    // int i = 0;
    while (status == TRUE)
    {
        // printf("Waiting for player ... %d\n", status);
        if(status == BATTLE || status == FALSE)break;
        puts("Waiting for player ...");
        sleep(1);
        // i++;
    } 
    return NULL;
}

void *receiveInput( void *ptr )
{
    char buffer[1024];
    memset(buffer, FALSE, sizeof(buffer));

    while((valread = read(sock, buffer, 1024))) {
        // clear();

        if(!strcmp(buffer, "win")) {
            printf("Game berakhir kamu menang\n");
            status = WIN;
            break;
        }

        if(!strcmp(buffer, "lose")) {
            printf("Game berakhir kamu kalah\n");
            status = LOSE;
            break;
        }

        printf("%s\n", buffer);
        // printf("%s\n", buffer);

        memset(buffer, FALSE, sizeof(buffer));
    }

    status = FALSE;
    return NULL;
}




int main(int argc, char const *argv[]) 
{ 
	int is_login = 0, flag = 0, iret; 
	struct sockaddr_in serv_addr; 
	char message[1024], buffer[1024] = {0}, c;
    pthread_t thread;

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
        memset(buffer, FALSE, sizeof(buffer));
        memset(message, FALSE, sizeof(message));

        // if(flag != BATTLE)clearscr();
        // else flag = FALSE;

        if(flag != FALSE) {
            if(flag == AUTHENTICATED){
                printf("Sudah Login\n");
            }else if(flag == LOGIN) {
                printf("Login Terlebih Dahulu\n");
            }
            flag = FALSE;
        }

        if(!is_login)printf("1.Login\n2.Register\nChoices : ");
        else printf("1.Find Match\n2.Logout\nChoices : ");
        
        gets(message);
        int input = checkInput(message);
        memset(message, FALSE, sizeof(message));

        switch (input)
        {
        case LOGIN:
            if(is_login){
                flag = AUTHENTICATED;
                printf("Input Salah\n");
                continue;
            }
            
            send(sock , "login" , strlen("login") , 0 ); 
            
            printf("Username : ");
            gets(message);
            // puts(message);
            send(sock , message , strlen(message) , 0 );
            memset(message, FALSE, sizeof(message));

            printf("Password : ");
            gets(message);
            // puts(message);
            send(sock , message , strlen(message) , 0 );
            memset(message, FALSE, sizeof(message));
            
            valread = read(sock, buffer, 1024);
            puts(buffer);
            if(!strcmp("login success", buffer)) is_login = TRUE;

            break;
        case REGISTER:
            if(is_login){
                flag = AUTHENTICATED;
                printf("Input Salah\n");
                continue;
            }

            send(sock , "register" , strlen("register") , 0 ); 
            
            printf("Username : ");
            gets(message);
            // puts(message);
            send(sock , message , strlen(message) , 0 );
            memset(message, FALSE, sizeof(message));

            printf("Password : ");
            gets(message);
            // puts(message);
            send(sock , message , strlen(message) , 0 );
            memset(message, FALSE, sizeof(message));

            valread = read(sock, buffer, 1024);
            puts(buffer);

            break;
        case FIND_MATCH:
            if(!is_login){
                flag = UNAUTHENTICATED;
                printf("Input Salah\n");
                continue;
            }

            status = TRUE;

            send(sock , "waiting for player" , strlen("waiting for player"), 0 );

            iret = pthread_create(&thread,NULL,&loop,NULL);

            if(iret)
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
                exit(EXIT_FAILURE);
            }

            while (valread = read(sock, buffer, 1024))
            {
                if(!strcmp(buffer,"found your opponent")){
                    status = FALSE;
                    break;
                }

                memset(buffer, FALSE, sizeof(buffer));
            }

            printf("Game dimulai silahkan tap tap secepat mungkin !!\n");

            sleep(1);

            status = BATTLE;

            iret = pthread_create(&thread,NULL,&receiveInput,NULL);

            if(iret)
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
                exit(EXIT_FAILURE);
            }

            set_keypress();

            while (status == BATTLE)
            {
                if(status != BATTLE) break;
                c = getc(stdin);
                if(status == WIN) break;
                if(c == 32) {
                    if(status != WIN) send(sock , "hit" , strlen("hit"), 0 );
                }
            }
            status = FALSE;

            reset_keypress();

            break;
        case LOGOUT:
            if(!is_login){
                flag = UNAUTHENTICATED;
                printf("Input Salah\n");
                continue;
            }
            
            is_login = FALSE;
            break;
        default:
            printf("Input Salah\n");
            break;
        }
    }
    
	return 0; 
} 
