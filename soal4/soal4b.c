// gcc -pthread -o soal4b soal4b.c
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define X1 4
#define X2 5

int result[X1][X2];
void *factorial( void *ptr );

int main()
{
    key_t key = 1234;
    int *res; 
    int i, j;

    int shmid = shmget(key, sizeof(int)*X1*X2, IPC_CREAT | 0666), iret[X1][X2];
    res = shmat(shmid, NULL, 0);

    pthread_t threads[X1][X2];

    for (i = 0; i < X1; i++)
    {
        for ( j = 0; j < X2 - 1; j++)
            printf("%-8d", res[j * X2 + i]);
        printf("%-8d\n", res[4 * X2 + i]);
    }
    
    for ( i = 0; i < X1; i++)
    {
        for ( j = 0; j < X2; j++)
            result[i][j] = 0;
    }
    

    for (i = 0; i < X1; i++)
    {
        for (j = 0; j < X2; j++)
        {
            int arr[3] = {res[j * X2 + i], i, j};

            iret[i][j] = pthread_create(&threads[i][j], NULL, factorial, arr);

            if(iret[i][j]) //jika eror
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i][j]);
                exit(EXIT_FAILURE);
            }

            pthread_join(threads[i][j], NULL);
        }
    }

    for (i = 0; i < X1; i++)
    {
        for ( j = 0; j < X2 - 1; j++)
            printf("%-8d", result[i][j]);
        printf("%-8d\n", result[i][X2 - 1]);
    }
    
    sleep(3);

    shmdt(res);
    shmctl(shmid, IPC_RMID, NULL);
}

void *factorial( void *ptr )
{
    int *param = (int *)ptr, i;

    if(param[0] < 1){
        result[param[1]][param[2]] = 0;
        return NULL;
    }

    result[param[1]][param[2]] = 0;

    for ( i = 1; i <= param[0]; i++)
    {
        result[param[1]][param[2]] += i;
    }

    return NULL;
}