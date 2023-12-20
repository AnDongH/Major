#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LEN 100
#define TOKEN_CNT 64

// 파이프 라인
void do_pipe(char *args[], int pipe_idx[], int pipe_cnt, int background)
{
  	int pipes[TOKEN_CNT][2] = {0, };  // 파이프 목록
  	int pid;
  	int status;
  	/*****  1번째 명령어 실행  *****/
  	pipe(pipes[0]);        // 파이프 생성
  	if ((pid = fork()) < 0) {
    	perror("Fork Error");
    	exit(0);
  	}
  	else if (pid == 0) {
    	close(STDOUT_FILENO);  // stdout close
    	dup2(pipes[0][1], STDOUT_FILENO);   // 0번째 pipe stdout에 복사
    	close(pipes[0][1]);    // pipe close
    	execvp(args[pipe_idx[0]], &args[pipe_idx[0]]);  // exec
    	printf("command not found | invalid option\n");
		exit(0);
  	}
  	close(pipes[0][1]);     // pipe close
  	wait(&status);          // exec 종료까지 대기
  	/*****  마지막 명령어 제외 모두 실행  *****/
  	for (int i = 0; i < pipe_cnt - 1; i++) {
    	pipe(pipes[i+1]);     // 파이프 생성
    	if ((pid = fork()) < 0) {
      		perror("Fork Error");
      		exit(0);
    	}
    	else if (pid == 0) {
      		close(STDIN_FILENO);  // stdin close
      		close(STDOUT_FILENO); // stdout close
      		dup2(pipes[i][0], STDIN_FILENO);    // pipe stdin에 복사
      		dup2(pipes[i+1][1], STDOUT_FILENO); // pipe stdout에 복사
      		close(pipes[i][0]);   // pipe close
      		close(pipes[i+1][1]); // pipe close
      		execvp(args[pipe_idx[i+1]], &args[pipe_idx[i+1]]);  // exec
      		printf("command not found | invalid option\n");
			exit(0);
    	}
    	close(pipes[i+1][1]); // pipe close
    	wait(&status);        // exec 종료까지 대기
  	}
  	/*****  마지막 명령어 실행  *****/
  	if ((pid = fork()) < 0) {
    	perror("Fork Error");
    	exit(0);
  	}
  	else if (pid == 0) {
    	close(STDIN_FILENO);  // stdin close
    	dup2(pipes[pipe_cnt-1][0], STDIN_FILENO); // pipe stdin에 복사
    	close(pipes[pipe_cnt-1][0]);  // pipe close
    	close(pipes[pipe_cnt-1][1]);  // pipe close
    	execvp(args[pipe_idx[pipe_cnt]], &args[pipe_idx[pipe_cnt]]);  // exec
		printf("command not found | invalid option\n");
		exit(0);
  	}
	close(pipes[pipe_cnt-1][1]);
	if (background == 0)
  		wait(&status);  // exec 종료까지 대기
	else printf("background process\n");
  	return;
}


int main(void) {

	// 시스템 변수
  	char *args[MAX_LEN / 2 + 1];
  	int should_run = 1;        
  	int background = 0;

	// 명령 변수
  	char *input;
  	int status;

	// 파이프 변수
	int pipe_idx[TOKEN_CNT] = {0,};
	int pipe_cnt = 0;
	int has_pipe = 0;

	// 무한 반복
  	while (should_run) {
		// 인터페이스 출력
    	printf("my_shell> ");

		// 출력 버퍼 비워두기
    	fflush(stdout);

		// 기본적으로 백그라운드, 파이프 비활성
    	background = 0;
		has_pipe = 0;
		pipe_cnt = 0;

    	// 명령어 입력
    	input = (char*)malloc(MAX_LEN*sizeof(char)); 
    	fgets(input, MAX_LEN, stdin);

		// 그냥 엔터만 치면 반복문 맨 끝으로 점프
    	if (strcmp(input, "\n") == 0){
      		goto next_time;
    	}
		// exit 치면 쉘 나가기
    	else if (strcmp(input, "exit\n") == 0){
      		break; 
    	}

    	// toknize
    	char *p;
    	p = strtok(input, " \n");
   
    	status = 0;
    	while(p != NULL){
      		args[status++] = p; 
      		p = strtok(NULL, " \n");
    	}
   
		// 마지막에 널문자
    	args[status] = NULL; 

   
    	// 문자열 비교 -> 백그라운드, 파이프 확인
    	for (int i = 0; i < status; i++){
      		if (strcmp(args[i], "&") == 0){
				background = 1;
        		args[status - 1] = NULL; 
      		}
			else if (strcmp(args[i], "|") == 0){
				args[i] = NULL;
				pipe_idx[++pipe_cnt] = i+1;
				has_pipe = 1;
			}
    	}


		// 내부 명령어 cd 추가 구현
		if (strcmp(args[0], "cd") == 0){
			if (chdir(args[1]) == -1){
				perror("cd");
				goto next_time;
			}
			else{
				goto next_time;
			}
		}
		
		// 파이프 하나라도 존재하면, 파이프 모드로 실행
		if (has_pipe == 1){
			do_pipe(args, pipe_idx, pipe_cnt, background);
			goto next_time;
		}


    	// 자식 프로세스 생성
    	pid_t pid = fork();
    	if (pid < 0) {
      		perror("Fork error");
      		exit(0);
    	}
		// 자식 프로세스이면, 파일 실행
    	else if (pid == 0){
      		execvp(args[0], args); 

			// 이 밑은 프로그램이 정상 작동하지 않았을때 실행
    		printf("command not found | invalid option\n");
			exit(0); 
    	}
		// 부모 프로세스이면
    	else{
			// 자식 프로세스 끝나길 대기
      		if (background == 0){
         		waitpid(pid, NULL, 0);
      		}
			// 백그라운드 모드
      		else{
        		printf("background process\n");
      		}  
 
     
    	}
   
next_time:
		free(input);
  	}
  	return 0;
}










