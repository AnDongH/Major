#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem;

void* threadFunc1(void *arg)
{
		sem_wait(&sem);

		printf("Thread 1 go to the toilet...\n");

		sleep(5);

		sem_post(&sem);
		printf("Thread 1 exits\n");

		return NULL;
}

void* threadFunc2(void *arg)
{
		sem_wait(&sem);

		printf("Thread 2 go to the toilet...\n");

		sleep(3);

		sem_post(&sem);
		printf("Thread 2 exits\n");

		return NULL;
}

void* threadFunc3(void *arg)
{
		sem_wait(&sem);

		printf("Thread 3 go to the toilet...\n");

		sleep(3);

		sem_post(&sem);
		printf("Thread 3 exits\n");

		return NULL;
}

int main(void)
{
    pthread_t tid1, tid2, tid3;

	// Semaphore init value = 2
    sem_init(&sem, 0, 2);
 
    pthread_create(&tid1, NULL, threadFunc1, NULL);
    pthread_create(&tid2, NULL, threadFunc2, NULL);
    pthread_create(&tid3, NULL, threadFunc3, NULL);
 
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);

	sleep(3);

    sem_destroy(&sem);
 
    printf("FINISHED\n");
	return 0;
}
