#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int sum; 
void *runner(void *param);

int main(int argc, char *argv[])
{
	pthread_t tid; 
	pthread_attr_t attr;

	// 공백 입력 or 다중 입력 예외 처리
	if (argc != 2) {
		fprintf(stderr, "usage: ./sum_pthread <integer value>\n");
		return -1;
	}

	// 음수 입력 예외 처리
	if (atoi(argv[1]) < 0) {
		fprintf(stderr, "input value : [%d] must be >= 0\n", atoi(argv[1]));
		return -1;
	}

	// 쓰레드 속성 객체 attr 초기화
	pthread_attr_init(&attr);

	/* create thread */
	// 쓰레드(tid)에서 argv[1]을 매개변수로 하여 runner 함수 실행
	pthread_create(&tid, &attr, runner, argv[1]);

	/* wait for all created threads */
	// 쓰레드(tid)가 종료될 때 까지 대기
	pthread_join(tid, NULL);

	printf("sum = %d\n", sum);
}

void *runner(void *param)
{
	int i, upper = atoi(param);
	sum = 0;

	for (i = 1; i <= upper; i++)
		sum += i;

	/* exit thread */
	// 쓰레드 종료
	pthread_exit(0);
}

