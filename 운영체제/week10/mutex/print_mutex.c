#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define N 3

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KBLU "\x1B[34m"

pthread_mutex_t mutex;

void *print_red(void *data) {
  while (1) {
		pthread_mutex_lock(&mutex);
		printf("%sZERG UNITS\n", KRED);
		printf("----------------\n");
		printf("Hydralisk\n");
		printf("Mutalisk\n");
		printf("Ultralisk\n");
		printf("----------------\n%s", KNRM);
		pthread_mutex_unlock(&mutex);
		usleep(rand()%2);
	}
}

void *print_blue(void *data) {
  while (1) {
		pthread_mutex_lock(&mutex);
		printf("%sTERRAN UNITS\n", KBLU);
		printf("----------------\n");
		printf("Marine\n");
		printf("Siege Tank\n");
		printf("Wraith\n");
		printf("----------------\n%s", KNRM);
		pthread_mutex_unlock(&mutex);
		usleep(rand()%2);
  }
}

void *print_green(void *data) {
  while (1) {
		pthread_mutex_lock(&mutex);
		printf("%sPROTOSS UNITS\n", KGRN);
		printf("----------------\n");
		printf("Zealot\n");
		printf("Dragoon\n");
		printf("Observer\n");
		printf("----------------\n%s", KNRM);
		pthread_mutex_unlock(&mutex);
		usleep(rand()%2);
  }
}

int main() {
		pthread_t thread_id[N];
		srand(time(NULL));
		
		pthread_mutex_init(&mutex, NULL);

		pthread_create(&thread_id[0], NULL, print_red, NULL);
		pthread_create(&thread_id[1], NULL, print_blue, NULL);
		pthread_create(&thread_id[2], NULL, print_green, NULL);
	
		for (int i = 0; i < N; i++) {
				pthread_join(thread_id[i], NULL);
		}
	
		pthread_mutex_destroy(&mutex);
}
