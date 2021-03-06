// gcc -pthread -o soal1_pokezone soal1_pokezone.c
#include <stdio.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <wait.h>
#include <time.h>
#include <sys/prctl.h>

//General
#define FALSE 0
#define TRUE 1

//Item
#define LULLABY_POWDER 0
#define POKEBALL 1
#define BERRY 2

// Shared memory index
#define LULLABY_STORE_NUM 0
#define POKEBALL_STORE_NUM 1
#define BERRY_STORE_NUM 2
#define INDEX_X_POKEMON 3
#define INDEX_Y_POKEMON 4
#define NEW_ESCAPE_RATE 5
#define NEW_CAPTURE_RATE 6
#define NEW_POKEDOLLAR 7
#define OLD_ESCAPE_RATE 8
#define OLD_CAPTURE_RATE 9
#define OLD_POKEDOLLAR 10
#define IS_LULLABY_USED 11

int escape_rate[] = {5, 10, 20};
int capture_rate[] = {70, 50, 30};
int pokedollar[] = {80, 100, 200};
int *shared_array;
int lullaby_check[100];
int count_lullaby_effect_running;
int first_escape_rate;

void * itemAdding(void * ptr) {
    int i;
    while (TRUE)
    {
        for ( i = 0; i < 30; i++)
        {
            if((shared_array[LULLABY_STORE_NUM] + shared_array[BERRY_STORE_NUM] + shared_array[POKEBALL_STORE_NUM]) >= 200) break;

            shared_array[i % 3] += 1;
        }

        sleep(10);
    }
    
    return NULL;
}

void * lullabyEffect(void * ptr) {
    int *param = (int *)ptr;

    int i, count = 0;

    if(count_lullaby_effect_running <= 0)
        first_escape_rate = shared_array[OLD_ESCAPE_RATE];
    
    count_lullaby_effect_running++;

    for ( i = 0; i < 20; i++)
    {
        if(shared_array[OLD_CAPTURE_RATE] >= 100) break;
        shared_array[OLD_CAPTURE_RATE] += 1;
        count++;
    }

    shared_array[OLD_ESCAPE_RATE] = 0;
    
    sleep(10);
        
    shared_array[OLD_CAPTURE_RATE] -= count;
    
    if(count_lullaby_effect_running == 1)
        shared_array[OLD_ESCAPE_RATE] = first_escape_rate;

    count_lullaby_effect_running--;

    lullaby_check[param[0]] = FALSE;

    return NULL;
}

void * randomPokemon( void * ptr) {
    while (TRUE)
    {
        int random_num = rand() % 100;
        if(random_num < 80) shared_array[INDEX_X_POKEMON] = 0;
        else if(random_num < 95) shared_array[INDEX_X_POKEMON] = 1;
        else shared_array[INDEX_X_POKEMON] = 2;

        random_num = rand() % 5;

        shared_array[INDEX_Y_POKEMON] = random_num;

        shared_array[NEW_ESCAPE_RATE] = escape_rate[shared_array[INDEX_X_POKEMON]];
        shared_array[NEW_CAPTURE_RATE] = capture_rate[shared_array[INDEX_X_POKEMON]];
        shared_array[NEW_POKEDOLLAR] = pokedollar[shared_array[INDEX_X_POKEMON]];

        random_num = rand() % 8000;
        if(random_num == 77) {
            shared_array[NEW_ESCAPE_RATE] += 5;
            shared_array[NEW_CAPTURE_RATE] -= 20;
            shared_array[NEW_POKEDOLLAR] += 5000;
        }

        sleep(1);
    }
    return NULL;
}

int main() {

    key_t key = 1234;
    int shmid = shmget(key, sizeof(int)*20, IPC_CREAT | 0666), i;
    shared_array = shmat(shmid, NULL, 0);
    int irett[2];
    count_lullaby_effect_running = 0;

    pthread_t item_adding;
    pthread_t lullaby_effect[100];
    pthread_t random_pokemon;

    for ( i = 0; i < 12; i++) shared_array[i] = 0;
    shared_array[POKEBALL_STORE_NUM] = 40;
    shared_array[LULLABY_STORE_NUM] = shared_array[BERRY_STORE_NUM] = 30;
    shared_array[IS_LULLABY_USED] = FALSE;

    irett[0] = pthread_create(&item_adding, NULL, itemAdding, NULL);
    irett[1] = pthread_create(&random_pokemon, NULL, randomPokemon, NULL);

    for ( i = 0; i < 100; i++) lullaby_check[i] = FALSE;
    
    pid_t checking_lullaby = fork();

    if(checking_lullaby == 0) {
        while (TRUE)
        {
            if(shared_array[IS_LULLABY_USED]) {
                for ( i = 0; i < 100; i++)
                {
                    if(!lullaby_check[i]) {
                        lullaby_check[i] = TRUE;

                        int iret;
                        int arr[1] = {i};

                        iret = pthread_create(&lullaby_effect[i], NULL, lullabyEffect, (void*) arr);
                        break;
                    }
                }
                
                shared_array[IS_LULLABY_USED] = FALSE;
            }
        }
    }

    char message[100];

    while (TRUE)
    {
        puts("Ketik shutdown untuk shutdown game");
        
        gets(message);

        int length = strlen(message);
        
        for ( i = 0; i < length; i++)
        {
            message[i] = tolower(message[i]);
        }

        if(!strcmp("shutdown", message)) break;
        puts("Input salah");   
    }

    pid_t shutdown = fork();

    if(shutdown == 0) {
        char *arr[] = {"pkill","-f","soal1_traizone",NULL};
        execv("/bin/pkill",arr);
    }

    shmdt(shared_array);
    shmctl(shmid, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}