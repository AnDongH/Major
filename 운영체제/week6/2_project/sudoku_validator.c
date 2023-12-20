#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define num_threads 27

// 각 쓰레드의 valid check가 유효한지 저장하기 위한 int list
// 0이면 invalid, 1이면 valid
int valid[num_threads] = {0};

typedef struct {
	int row;
	int column;		
} parameters;

// Sudoku puzzle to be solved
int sudoku[9][9] = {
	{6, 2, 4, 5, 3, 9, 1, 8, 7},
	{5, 1, 9, 7, 2, 8, 6, 3, 4},
	{8, 3, 7, 6, 1, 4, 2, 9, 5},
	{1, 4, 3, 8, 6, 5, 7, 2, 9},
	{9, 5, 8, 2, 4, 7, 3, 6, 1},
	{7, 6, 2, 3, 9, 1, 4, 5, 8},
	{3, 7, 1, 9, 5, 6, 8, 4, 2},
	{4, 9, 6, 1, 8, 2, 5, 7, 3},
	{1, 8, 5, 4, 7, 3, 9, 1, 6}
};

// Method that determines if numbers 1-9 only appear once in a column
void *isColumnValid(void* param) {
	// Confirm that parameters indicate a valid col subsection
	parameters *params = (parameters*)param;
	int row = params->row;
	int col = params->column;		

	// 아래 두 경우에 스레드를 종료시킴.
	// 1. column이 유효한지 살필 것이기 때문에 col의 값이 8초과이거나
	// 2. row가 0이 아닐 경우 (row == 0 부터 row == 8 까지 값을 확인할 것이기에)
	if (row != 0 || col > 8) {
		fprintf(stderr, "Invalid row or column for col subsection! row=%d, col=%d\n", row, col);
		/* TODO 1 : exit thread */
	}

	// validityArray는 1~9의 숫자의 존재 유무를 판단하기 위해 만듦
	int validityArray[9] = {0};
	// 한 행내에서 row만을 바꿔가며 1~9가 모두 있는지 확인 
	int i;	
	for (i = 0; i < 9; i++) {
		int num = sudoku[i][col];
		// 같은 숫자가 이미 존재할 경우
		if (num < 1 || num > 9 || validityArray[num - 1] == 1) {
			/* TODO 2 : exit thread */
		// 처음 나온 숫자일 경우
		} else {
			validityArray[num - 1] = 1;		
		}
	}

	// If reached this point, col subsection is valid.
	valid[18 + col] = 1;
	/* TODO 3 : exit thread */
}

// Method that determines if numbers 1-9 only appear once in a row
void *isRowValid(void* param) {
	// Confirm that parameters indicate a valid row subsection
	parameters *params = (parameters*) param;
	int row = params->row;
	int col = params->column;		

	// 아래 두 경우에 스레드를 종료시킴.
	// 1. row가 유효한지 살필 것이기 때문에 row의 값이 8초과이거나
	// 2. col이 0이 아닐 경우 (col == 0 부터 col == 8 까지 값을 확인할 것이기에)
	if (col != 0 || row > 8) {
		fprintf(stderr, "Invalid row or column for row subsection! row=%d, col=%d\n", row, col);
		/* TODO 4 : exit thread */
	}

	// validityArray는 1~9의 숫자의 존재 유무를 판단하기 위해 만듦
	int validityArray[9] = {0};
	// 한 열 내에서 col만을 바꿔가며 1~9가 모두 있는지 확인 
	int i;
	for (i = 0; i < 9; i++) {
		// If the corresponding index for the number is set to 1, and the number is encountered again,
		// the valid array will not be updated and the thread will exit.
		int num = sudoku[row][i];
		if (num < 1 || num > 9 || validityArray[num - 1] == 1) {
			/* TODO 5 : exit thread */
		} else {
			validityArray[num - 1] = 1;		
		}
	}

	// If reached this point, row subsection is valid.
	valid[9 + row] = 1;
	/* TODO 6 : exit thread */
}

// Method that determines if numbers 1-9 only appear once in a 3x3 subsection
void *is3x3Valid(void* param) {
	// Confirm that parameters indicate a valid 3x3 subsection
	parameters *params = (parameters*) param;
	int row = params->row;
	int col = params->column;		

	// 아래 경우에 스레드를 종료시킴.
	// 1. row의 값이 6초과이거나
	// 2. col의 값이 6초과이거나
	// 3. row % 3의 값이 0이 아니거나
	// 4. col % 3의 값이 0이 아니거나
	if (row > 6 || row % 3 != 0 || col > 6 || col % 3 != 0) {
		fprintf(stderr, "Invalid row or column for subsection! row=%d, col=%d\n", row, col);
		/* TODO 7 : exit thread */
	}

	// validityArray는 1~9의 숫자의 존재 유무를 판단하기 위해 만듦
	int validityArray[9] = {0};
	// 한 3x3 matrix 단위 내에 1~9가 모두 있는지 확인 
	int i, j;
	for (i = row; i < row + 3; i++) {
		for (j = col; j < col + 3; j++) {
			int num = sudoku[i][j];
			if (num < 1 || num > 9 || validityArray[num - 1] == 1) {
				/* TODO 8 : exit thread */
			} else {
				validityArray[num - 1] = 1;		
			}
		}
	}
	// If reached this point, 3x3 subsection is valid.
	valid[row + col/3] = 1; // Maps the subsection to an index in the first 8 indices of the valid array
	/* TODO 9 : exit thread */
}

int main() {	
	// 생성한 스레드를 모아두기 위한 pthread_t 리스트
	pthread_t threads[num_threads];
	
	int threadIndex = 0;	
	int i,j;

	// Create 9 threads for 9 3x3 subsections, 9 threads for 9 columns and 9 threads for 9 rows.
	// This will end up with a total of 27 threads.
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {						
			// THREAD: 3x3 matrix check 
			if (i%3 == 0 && j%3 == 0) {
				parameters *data = (parameters *) malloc(sizeof(parameters));	
				data->row = i;		
				data->column = j;
				/* TODO 10 : create thread */
				// threads 리스트의 threadIndex위치에 쓰레드를 생성
				printf("%2dth thread created with \'is3x3Valid\' function at \t[%d][%d]\n", threadIndex, i, j);
			}
			// THREAD: column check
			if (i == 0) {
				parameters *columnData = (parameters *) malloc(sizeof(parameters));	
				columnData->row = i;		
				columnData->column = j;
				/* TODO 11 : create thread */
				// threads 리스트의 threadIndex위치에 쓰레드를 생성
				printf("%2dth thread created with \'isColumnValid\' function at \t[%d][%d]\n", threadIndex, i, j);
			}
			// THREAD: row checking
			if (j == 0) {
				parameters *rowData = (parameters *) malloc(sizeof(parameters));	
				rowData->row = i;		
				rowData->column = j;
				/* TODO 12 : create thread */
				// threads 리스트의 threadIndex위치에 쓰레드를 생성
				printf("%2dth thread created with \'isRowValid\' function at \t[%d][%d]\n", threadIndex, i, j);
			}				
		}
	}

	for (i = 0; i < num_threads; i++) {
		/* TODO 13 : wait for all created threads */
		printf("%2dth thread terminated\n", i);
	}

	// If any of the entries in the valid array are 0, then the sudoku solution is invalid
	for (i = 0; i < num_threads; i++) {
		if (valid[i] == 0) {
			printf("Sudoku solution is invalid!\n");
			return EXIT_SUCCESS;
		}
	}
	printf("Sudoku solution is valid!\n");
	return EXIT_SUCCESS;
}
