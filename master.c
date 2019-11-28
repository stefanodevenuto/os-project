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
#include "enviroment.h"
#include "semaphore.h"
#include "chessboard.h"

int main(int argc, char const *argv[]){

    int chessboard_rows;
    int chessboard_cols;

    int processes_number;
    int index_child;
    /*char buffer[2];*/
    
    int num_bytes;
    int parameters_id;
    int * parameters;


	pid_t select;
	int rows;
  	int columns;

	char * chessboard;


	int i,j;

	char y;

    int chessboard_mem_id;
    int chessboard_sem_id;

    int master_sem_id;
	
    /*int semid;
    char my_string[100];*/

    /* Setting the enviroment variable 
    set_env();

    /* Unset the buffering of the streams stdout and stderr*/
    setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	
	/* Get and parse of the set enviroment */
	
    parameters_id = shmget(PARAMETERS_MEM_KEY,10, 0666 | IPC_CREAT);
    parameters = shmat(parameters_id,NULL,0);

    parameters[0] = NUM_G;
    parameters[1] = NUM_P;
    parameters[2] = MAX_TIME;
    parameters[3] = BASE;
    parameters[4] = ALTEZZA;
    parameters[5] = FLAG_MIN;
    parameters[6] = FLAG_MAX;
    parameters[7] = ROUND_SCORE;
    parameters[8] = N_MOVES;
    parameters[9] = MIN_HOLD_NSEC;

    set_mem_chessboard();
    /*print_chessboard();*/
    


    printf("player number: %d\n", parameters[SO_NUM_G]);
	/*Creation of the Players*/
    for (index_child = 0; index_child < parameters[SO_NUM_G]; index_child++){
    	switch(fork()){
    		case -1:
    			fprintf(stderr, "Failed to Fork Players%s\n");
    			exit(EXIT_FAILURE);
    		case 0:
    			if(execve("./player", NULL, NULL)){
    				fprintf(stderr, "Execve() failed #%d : %s\n", errno, strerror(errno));
    				exit(EXIT_FAILURE);
    			}
			default:
				break;
    	}
    }

    /*printf("parameters[SO_NUM_G]: %d\n", parameters[SO_NUM_G]);*/
    
    master_sem_id = semget(KEY_SEM_MASTER_WAIT_PLRS, 1, 0666 | IPC_CREAT);
    
    sem_set_val(master_sem_id, READY_ENTRY, parameters[SO_NUM_G]);
    

    /*printf("Ready PADRE: %d\n",semctl(sem_id, READY_ENTRY, GETVAL));*/
    printf("Waiting on KEY: %d...\n", KEY_SEM_MASTER_WAIT_PLRS);

    sem_reserve_0(master_sem_id, 0);
    /*printf("Ready PADRE: %d\n",semctl(sem_id, READY_ENTRY, GETVAL));*/
    fprintf(stdout,"Player Synchronized\n");
    
    /* Wait the dead children*/
    while((select = wait(NULL)) != -1);
		/*printf("Process %d\n", select);*/

	
    /*


    Create Shared chessboard


    */

    /*printf("player number: %d\n", parameters[SO_NUM_G]);
    printf("pawn number: %d\n", parameters[SO_NUM_P]);
    printf("max time : %d\n", parameters[SO_MAX_TIME]);
    printf("base: %d\n", parameters[SO_BASE]);
    printf("height : %d\n", parameters[SO_ALTEZZA]);
    printf("min flag : %d\n", parameters[SO_FLAG_MIN]);
    printf("max flag : %d\n", parameters[SO_FLAG_MAX]);
    printf("total falg score : %d\n", parameters[SO_ROUND_SCORE]);
    printf("moves number of pawns : %d\n", parameters[SO_N_MOVES]);
    
    /*printf("%d\n", chessboard_base);
    printf("%d\n", chessboard_height);*/

    /* De-Allocate the chessboard */
    
	return 0;
}