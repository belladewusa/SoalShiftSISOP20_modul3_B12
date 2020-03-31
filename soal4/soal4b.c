#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define X1 4
#define X2 5

void *factorial( void *ptr );

void main()
{
    key_t key = 1234;
    unsigned long long int *res; 
    int i, j;

    int shmid = shmget(key, sizeof(unsigned long long int)*X1*X2, IPC_CREAT | 0666), iret[X1][X2];
    res = shmat(shmid, NULL, 0);

    pthread_t threads[X1][X2];
    
    for (i = 0; i < X1; i++)
    {
        for (j = 0; j < X2; j++)
        {
            void *ress;
            iret[i][j] = pthread_create(&threads[i][j], NULL, factorial, (void*) res[j * X2 + i]);
            printf("%d %d %d-> %llu -> SHMID\n",i*X1+j, i, j,res[j * X2 + i]);

            if(iret[i][j]) //jika eror
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i][j]);
                exit(EXIT_FAILURE);
            }

            pthread_join(threads[i][j], &ress);

            // printf("Pthread -> %llu\n", (long long int)ress);
            
            res[j * X2 + i] = (unsigned long long int)ress;
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
    sleep(5);

    shmdt(res);
    shmctl(shmid, IPC_RMID, NULL);
}

void *factorial( void *ptr )
{
    int ress = (int *)ptr, i;
    unsigned long long int res=1;
    if(ress < 1) return 1;
    for ( i = 1; i < ress; i++)
    {
        res *= i;
    }
    
    // printf("%d - Function -> %llu\n",ress, res);
    return (void *) res;
}