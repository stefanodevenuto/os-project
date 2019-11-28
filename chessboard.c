#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "semaphore.h"
#include "enviroment.h"


int pos(int i, int j){
	int parameters_id;
	int * parameters;
	int columns;
	parameters_id = shmget(PARAMETERS_MEM_KEY,10, 0666 | IPC_CREAT);
    parameters = shmat(parameters_id,NULL,0);
    columns = parameters[SO_BASE];
    shmdt(parameters);

    return i * columns + j;
}

void set_mem_chessboard(){
	int parameters_id;
	int * parameters;
	int chessboard_mem_id;
	int chessboard_sem_id;
	char * chessboard;
	int i;
	int j;
	int rows;
	int columns;

	

	parameters_id = shmget(PARAMETERS_MEM_KEY,10, 0666 | IPC_CREAT);
    parameters = shmat(parameters_id,NULL,0);

    rows = parameters[SO_ALTEZZA];
	columns = parameters[SO_BASE];

    chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(char) * rows * columns, 0666 | IPC_CREAT);
    chessboard = shmat(chessboard_mem_id,NULL,0);

    chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, rows * columns, 0666 | IPC_CREAT);

	for(i=0; i<rows; i++){
        for(j=0; j<columns; j++){
            chessboard[pos(i,j)] = 'A';
            semctl(chessboard_sem_id, pos(i,j), SETVAL, 1);
        }
    }

    shmdt(parameters);
    shmdt(chessboard);
}

void print_chessboard(){
	int parameters_id;
	int * parameters;
	int chessboard_mem_id;
	int chessboard_sem_id;
	char * chessboard;

	int i;
	int j;
	int rows;
	int columns;

	

	parameters_id = shmget(PARAMETERS_MEM_KEY,10, 0666 | IPC_CREAT);
    parameters = shmat(parameters_id,NULL,0);

    rows = parameters[SO_ALTEZZA];
	columns = parameters[SO_BASE];

    chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(char) * rows * columns, 0666 | IPC_CREAT);
    chessboard = shmat(chessboard_mem_id,NULL,0);

    chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, rows * columns, 0666 | IPC_CREAT);

	printf("Values:\n");
    for(i=0; i<rows; i++){
        for(j=0; j<columns; j++){
            printf("%d ", chessboard[pos(i,j)]);
        }
        printf("\n");
    }
    printf("Semaphore values:\n");
    for(i=0; i<rows; i++){
        for(j=0; j<columns; j++){
            printf("%d ", semctl(chessboard_sem_id, pos(i,j), GETVAL));

        }
        printf("\n");
    }

    shmdt(parameters);
    shmdt(chessboard);
}

