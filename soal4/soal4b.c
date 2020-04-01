#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define X1 4
#define X2 5

long double result[X1][X2];
void *factorial( void *ptr );

int main()
{
    key_t key = 1234;
    unsigned long long int *res; 
    int i, j;

    int shmid = shmget(key, sizeof(unsigned long long int)*X1*X2, IPC_CREAT | 0666), iret[X1][X2];
    res = shmat(shmid, NULL, 0);

    pthread_t threads[X1][X2];
    
    for ( i = 0; i < X1; i++)
    {
        for ( j = 0; j < X2; j++)
        {
            result[i][j] = 0.0;
        }
        
    }
    

    for (i = 0; i < X1; i++)
    {
        for (j = 0; j < X2; j++)
        {
            int arr[3] = {res[j * X2 + i], i, j};
            iret[i][j] = pthread_create(&threads[i][j], NULL, factorial, arr);
            // printf("%d %d %d-> %llu -> SHMID\n",i*X1+j, i, j,res[j * X2 + i]);

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
        {   
            printf("%.0Lf\t", result[i][j]);
        }
        printf("%.0Lf\n", result[i][X2 - 1]);
    }
    
    sleep(5);

    shmdt(res);
    shmctl(shmid, IPC_RMID, NULL);
}

void *factorial( void *ptr )
{
    int *param = (int *)ptr, i;
    // printf("%d\n", param[0]);
    if(param[0] < 1){
        result[param[1]][param[2]] = 1;
    }
    result[param[1]][param[2]] = 1;
    for ( i = 1; i < param[0]; i++)
    {
        result[param[1]][param[2]] *= i;
    }
}