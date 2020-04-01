//gcc -pthread -o [output] input.c
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define X1 4
#define X2 5
#define Y 2

void *multiply( void *ptr );

void main(int argc, char *argv[])
{
    key_t key = 1234;
    unsigned long long int *res;
    int i, j, k;

    int shmid = shmget(key, sizeof(unsigned long long int)*X1*X2, IPC_CREAT | 0666);
    res = shmat(shmid, NULL, 0);

    pthread_t threads[X1][X2][Y];
    int  iret[X1][X2][Y], number1[X1][Y]= {
            {3, 3}, 
            {3, 3}, 
            {3, 3},
            {3, 3}
        }, 
        number2[Y][X2] = {
            {3, 3, 3, 3, 3}, 
            {3, 3, 3, 3, 3}
        };

    for (i = 0; i < X1; i++)
    {
        for (j = 0; j < X2; j++)
        {
            res[i* X1 + j] = 0;
        }
    }
    
    for (i = 0; i < X1; i++)
    {
        for (k = 0; k < X2; k++)
        {
            for (j = 0; j < Y; j++)
            {
                void *ress;
                int arr[2] = {number1[i][j], number2[j][k]};

                iret[i][k][j] = pthread_create(&threads[i][k][j], NULL, multiply, (void*) arr);
                
                if(iret[i][k][j]) //jika eror
                {
                    fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i][k][j]);
                    exit(EXIT_FAILURE);
                }
                
                pthread_join(threads[i][k][j], &ress);
                printf("%d %d - %d %d -> %d %llu\n", i, j, j, k, (int)ress, res[k* X2 + i]);
                res[k*X2 + i] += (int)ress;
            }    
        }
    }

    for (i = 0; i < X1; i++)
    {
        for ( j = 0; j < X2 - 1; j++)
        {
            printf("%llu ", res[j * X2 + i]);
        }
        printf("%llu\n", res[j * X2 + X1 - 1]);
    }

    sleep(10);

    shmdt(res);
    shmctl(shmid, IPC_RMID, NULL);
}

void *multiply( void *ptr )
{
    int ress;
    int *param = (int *)ptr;
    ress = param[0]*param[1];
    // printf("%d %d - Function -> %d\n",param[0], param[1], ress);
    return (void *) ress;
}