#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include "enviroment.h"
#include "semaphore.h"

#define READY 0

int main(int argc, char const *argv[]){

    int chessboard_rows;
    int chessboard_cols;
    int processes_number;
    int index_child;
    char buffer[2];
    char *chessboard;
    int num_bytes;
    int * parameters;
    int sem_id;

	pid_t select;

	
    /*int semid;
    char my_string[100];*/

    /* Setting the enviroment variable 
    set_env();

    /* Unset the buffering of the streams stdout and stderr*/
    setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	
	/* Get and parse of the set enviroment */
	set_env();
    parameters = get_env();

    printf("player number: %d\n", parameters[SO_NUM_G]);

    chessboard = (char *)malloc(parameters[SO_BASE] * parameters[SO_ALTEZZA] * sizeof(chessboard));

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
    
    sem_id = semget(KEY, 1, 0666 | IPC_CREAT);
    sem_set_val(sem_id, READY, parameters[SO_NUM_G]);

    /*printf("Ready PADRE: %d\n",semctl(sem_id, READY, GETVAL));*/
    printf("Waiting on KEY: %d...\n", KEY);
    sem_reserve_0(sem_id, READY);
    /*printf("Ready PADRE: %d\n",semctl(sem_id, READY, GETVAL));*/
    fprintf(stdout,"Player Synchronized\n");
    
    /* Wait the dead children*/
    while((select = wait(NULL)) != -1);
		/*printf("Process %d\n", select);*/

	
    
    for(chessboard_cols=0;chessboard_cols<parameters[SO_BASE];chessboard_cols++){
    	for(chessboard_rows=0;chessboard_rows<parameters[SO_ALTEZZA];chessboard_rows++){
    		chessboard[ chessboard_cols * parameters[SO_ALTEZZA] + chessboard_rows ] = '0';
    	}
    }

    /*for(chessboard_cols=0;chessboard_cols<parameters[SO_BASE];chessboard_cols++){
    	for(chessboard_rows=0;chessboard_rows<parameters[SO_ALTEZZA];chessboard_rows++){
    		printf("%c ", chessboard[ chessboard_cols * parameters[SO_ALTEZZA] + chessboard_rows ]);
    	}
    	printf("\n");
    }

	
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
    free(chessboard);
    free(parameters);
	return 0;
}