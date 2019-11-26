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
#define KEY 13579

int main(int argc, char const *argv[]){


    int player_number;
    int pawn_number;
    int max_time;
    int chessboard_base;
    int chessboard_height;
    int min_flag;
    int max_flag;
    int total_flag_score;
    int moves_number_of_pawns;
    int chessboard_rows;
    int chessboard_cols;
    int processes_number;
    int index_child;
    char buffer[5];
    char *chessboard;
    int num_bytes;


    /* Creating a pipe where ONLY THE MASTER WRITE*/
    int pipe_master[2];

	/* Creating a pipe where ONLY THE MASTER WRITE*/
	int pipe_player[2];

	pid_t select;

	pipe(pipe_master);
	pipe(pipe_player);

    

    /*int semid;
    char my_string[100];*/

    /* Setting the enviroment variable */
    set_env();

    /* Unset the buffering of the streams stdout and stderr*/
    setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	
	/* Get and parse of the set enviroment */
    player_number = atol(getenv("SO_NUM_G"));
    pawn_number = atol(getenv("SO_NUM_P"));
    max_time = atol(getenv("SO_MAX_TIME"));
    chessboard_base = atol(getenv("SO_BASE"));
    chessboard_height = atol(getenv("SO_ALTEZZA"));
    min_flag = atol(getenv("SO_FLAG_MIN"));
    max_flag = atol(getenv("SO_FLAG_MAX"));
    total_flag_score = atol(getenv("SO_ROUND_SCORE"));
    moves_number_of_pawns = atol(getenv("SO_N_MOVES"));


    /* Initialization of the chessboard*/
    chessboard = (char *)malloc(chessboard_base * chessboard_height * sizeof(chessboard));

	/* Creation of the Players*/
    for (index_child = 0; index_child < player_number; index_child++){
    	switch(fork()){
    		case -1:
    			fprintf(stderr, "Failed to Fork Players%s\n");
    			exit(EXIT_FAILURE);
    		case 0:
    			/* CLOSE of WRITE in the MASTER PIPE,
    			   because the player read on READ of pipe_master*/
    			close(pipe_master[1]);
    			/* CLOSE of READ in the PLAYER PIPE
    			   because the player read on READ of pipe_master*/
    			close(pipe_player[0]);
    			
    			
    			buffer[0] = 'A';
    			/* Sending of a READY message*/
    			num_bytes =  write(pipe_player[1], buffer, sizeof(buffer));
    			printf("Scrittura da PLAYER: %d\n",num_bytes );
			
				close(pipe_player[1]);
				for (;;)
				{
					/* code */
				}
    	}
    }

    /* CLOSE of READ in the MASTER PIPE,
       because the master read on READ of pipe_player*/
    close(pipe_master[0]);
    /* CLOSE of WRITE in the PLAYER PIPE,
       because the master read on READ of pipe_player*/
	close(pipe_player[1]);

	printf("I'm waiting..\n");

	/* Waiting of the Players Message to Sincronyze the Game*/
	while(num_bytes = read(pipe_player[0], buffer, sizeof(buffer))){
    	printf("Lettura da MASTER: %d\n", num_bytes);
    	printf("%c\n", buffer[0]);
    }

    printf("Players Sinchronyzed\n");
    
    /* Wait the dead children*/
    while((select = wait(NULL)) != -1) {
		printf("Process %d\n", select);

	}
    
    for(chessboard_cols=0;chessboard_cols<chessboard_base;chessboard_cols++){
    	for(chessboard_rows=0;chessboard_rows<chessboard_height;chessboard_rows++){
    		chessboard[ chessboard_cols * chessboard_height + chessboard_rows ] = '0';
    	}
    }

    for(chessboard_cols=0;chessboard_cols<chessboard_base;chessboard_cols++){
    	for(chessboard_rows=0;chessboard_rows<chessboard_height;chessboard_rows++){
    		printf("%c ", chessboard[ chessboard_cols * chessboard_height + chessboard_rows ]);
    	}
    	printf("\n");
    }

	
    /*


    Create Shared chessboard


    */

    /*printf("player number: %d\n", player_number);
    printf("pawn number: %d\n", pawn_number);
    printf("max time : %d\n", max_time);
    printf("base: %d\n", chessboard_base);
    printf("height : %d\n", chessboard_height);
    printf("min flag : %d\n", min_flag);
    printf("max flag : %d\n", max_flag);
    printf("total falg score : %d\n", total_flag_score);
    printf("moves number of pawns : %d\n", moves_number_of_pawns);
    
    /*printf("%d\n", chessboard_base);
    printf("%d\n", chessboard_height);*/

    /* De-Allocate the chessboard*/
    free(chessboard);
	return 0;
}