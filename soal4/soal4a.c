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

int main()
{
    key_t key = 1234;
    int *res;
    int i, j, k;

    int shmid = shmget(key, sizeof(int)*X1*X2, IPC_CREAT | 0666);
    res = shmat(shmid, NULL, 0);

    pthread_t threads[X1][X2][Y];
    int  iret[X1][X2][Y], number1[X1][Y]= {
            {20, 20}, 
            {20, 20}, 
            {20, 20},
            {20, 20}
        }, 
        number2[Y][X2] = {
            {20, 20, 20, 20, 20}, 
            {20, 20, 20, 20, 20}
        };

    for (i = 0; i < X1; i++)
    {
        for (j = 0; j < X2; j++)
        {
            res[j* X2 + i] = 0;
        }
    }
    
    for (i = 0; i < X1; i++)
    {
        for (k = 0; k < X2; k++)
        {
            for (j = 0; j < Y; j++)
            {
                int arr[2] = {number1[i][j], number2[j][k]};

                iret[i][k][j] = pthread_create(&threads[i][k][j], NULL, multiply, (void*) arr);
                
                if(iret[i][k][j]) //jika eror
                {
                    fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i][k][j]);
                    exit(EXIT_FAILURE);
                }
                
            }    
        }
    }

    for ( i = 0; i < X1; i++)
    {
        for ( k = 0; k < X2; k++)
        {
            for ( j = 0; j < Y; j++)
            {
                void *ress;
                pthread_join(threads[i][k][j], &ress);
                // printf("%d %d - %d %d -> %d %llu\n", i, j, j, k, (int)ress, res[k* X2 + i]);
                res[k*X2 + i] += (int)ress;
            }
        }
    }
    

    for (i = 0; i < X1; i++)
    {
        for ( j = 0; j < X2 - 1; j++)
            printf("%-8d", res[j * X2 + i]);
        printf("%-8d\n", res[j * X2 + X1 - 1]);
    }

    sleep(3);

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