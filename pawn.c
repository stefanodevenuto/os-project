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
#include <sys/msg.h>
#include "semaphore.h"

int main(int argc, char *argv[]){
	int i,j,switch_color_pawn;
	int parameters_id;
	struct param * parameters;
	int player_sem_id;
	int chessboard_mem_id;
	int * chessboard;
	int chessboard_sem_id;

	int player_msg_id;
	struct message message_to_pawn;
	struct strategy strategy_pawn;
	long type;
	int player_letter;
	int a;
	int master_sem_id;

	int rows;
	int columns;
	int new_y_pos;
	int new_x_pos;
	int x_pos;
	int y_pos;
	int test;
	char * strategy;

	parameters_id = atol(argv[1]);
	type = atol(argv[2]);
	player_letter = -atol(argv[3]);

	printf("SONO UNA PAWN\n");



		  /* Getting chessboard, player semaphore and player queue*/
	/* ---------------------------------------------------------------- */
	parameters = shmat(parameters_id,NULL,0);
	player_sem_id = semget(getppid(), 1, 0666);
	player_msg_id = msgget(getppid(), 0666);
	/* ---------------------------------------------------------------- */

	rows = parameters->SO_ALTEZZA;
	columns = parameters->SO_BASE;
		     /* Receiving coordinates and accessing chessboard*/
	/* ---------------------------------------------------------------- */
	/*printf("STO PER RICEVERE LE COORDINATE\n");*/
	a = msgrcv(player_msg_id, &message_to_pawn, LEN_X_Y, type, 0);
	if(a == -1){
		fprintf(stderr, "MSGRCV: ret: %d, errno: %d, %s\n", a, errno, strerror(errno));
	}
	
	if((chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(int) * parameters->SO_ALTEZZA * parameters->SO_BASE, 0666)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed access to memory for pawns\n");
		exit(EXIT_FAILURE);
	}

	chessboard = shmat(chessboard_mem_id,NULL,0);

	if((chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, rows * columns, 0666)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed access to sem memory for pawns\n");
		exit(EXIT_FAILURE);
	}
	/* ---------------------------------------------------------------- */

	y_pos = message_to_pawn.y;
	x_pos = message_to_pawn.x;

					/* Unblock players */
    /* ----------------------------------------------------------------- */
	sem_reserve_1(player_sem_id, 0);
    /* ----------------------------------------------------------------- */


					/* Wait for strategy */
    /* ----------------------------------------------------------------- */
    /*printf("STO PER LEGGERE LA STRATEGIA\n");*/
	a = msgrcv(player_msg_id, &strategy_pawn,sizeof(char) * STRAT_LEN, type, 0);
	if(a == -1){
		fprintf(stderr, "MSGRCV: ret: %d, errno: %d, %s\n", a, errno, strerror(errno));
	}
	/*printf("Pedina riceve strategia: %s\n", strategy_pawn.strategy);*/
    /* ----------------------------------------------------------------- */



	/*
	.
	.
	. Gestione strategia ricevuta
	.
	.
	.
	*/

	strategy = (char * )malloc(sizeof(char) * strlen(strategy_pawn.strategy));
	
					/* Unblock players */
    /* ----------------------------------------------------------------- */
	sem_reserve_1(player_sem_id, 0);
    /* ----------------------------------------------------------------- */
	


					/* Wait for 1 on synchro */
    /* ----------------------------------------------------------------- */
    if((master_sem_id = semget(MAIN_SEM, 3, 0666)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed access to memory for pawns\n");
		exit(EXIT_FAILURE);
	}
	
	sem_reserve_1(master_sem_id, SYNCHRO);
    /* ----------------------------------------------------------------- */

	/*
	.
	.
	.
	. GAME
	.
	.
	.
	*/
					/* Prova spostamenti */
	/* ------------------------------------------------------------------ */
	/* NORTH */
	/*for(i = 0; i < strlen(strategy_pawn.strategy); i++){
		switch(strategy_pawn.strategy[i]){
			case 'N':
				new_y_pos = y_pos - 1;
				new_x_pos = x_pos;
				if(new_y_pos < 0){
					printf("Uscito da NORTH\n");
				}
				break;

			case 'E':
				new_y_pos = y_pos;
				new_x_pos = x_pos + 1;
				if(new_x_pos > columns - 1){
					printf("Uscito da WEST\n");
				}
				break;

			case 'S':
				new_y_pos = y_pos + 1;
				new_x_pos = x_pos;
				if(new_y_pos > rows - 1){
					printf("Uscito da SOUTH\n");
				}
				break;
				
			case 'W':
				new_y_pos = y_pos;
				new_x_pos = x_pos - 1;
				if(new_x_pos < 0){
					printf("Uscito da WEST\n");
				}
				break;
		}
		test = sem_reserve_1_no_wait(chessboard_sem_id, new_y_pos * columns + new_x_pos);
		if(test == -1){
			printf("FALLIMENTO EAST\n");
		}else{
			chessboard[y_pos * columns + x_pos] = 0;
			sem_release(chessboard_sem_id, y_pos * columns + x_pos);
					
			chessboard[new_y_pos * columns + new_x_pos] = player_letter;

			y_pos = new_y_pos;
			x_pos = new_x_pos;		
		}
		
	}*/
	free(strategy);
	/* ------------------------------------------------------------------ */
	

	exit(EXIT_SUCCESS);

}


