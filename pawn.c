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
	int parameters_id;
	struct param * parameters;
	int player_sem_id;
	int chessboard_mem_id;
	int * chessboard;
	int chessboard_sem_id;

	int player_msg_id;
	struct message message_to_pawn;
	long type;
	int player_letter;
	int a;
	int master_sem_id;

	parameters_id = atol(argv[1]);
	type = atol(argv[2]);
	player_letter = -atol(argv[3]);

	
	parameters = shmat(parameters_id,NULL,0);

	player_sem_id = semget(getppid(), 1, 0666);

	player_msg_id = msgget(getppid(), 0666);

	a = msgrcv(player_msg_id, &message_to_pawn, LEN_X_Y, type, 0);
	if(a == -1){
		fprintf(stderr, "MSGRCV: ret: %d, errno: %d, %s\n", a, errno, strerror(errno));
	}

	

    
	if((chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(int) * parameters->SO_ALTEZZA * parameters->SO_BASE, 0666)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed access to memory for pawns\n");
		exit(EXIT_FAILURE);
	}

					/* Unblock players */
    /* ----------------------------------------------------------------- */
	sem_reserve_1(player_sem_id, 0);
    /* ----------------------------------------------------------------- */


					/* Wait for strategy */
    /* ----------------------------------------------------------------- */
    //printf("ASPETTO LA STRATEGIA\n");
	a = msgrcv(player_msg_id, &message_to_pawn, LEN_X_Y, type, 0);
	if(a == -1){
		fprintf(stderr, "MSGRCV: ret: %d, errno: %d, %s\n", a, errno, strerror(errno));
	}
	//printf("STRATEGIA RICEVUTA\n");
    /* ----------------------------------------------------------------- */



	/*
	.
	.
	. Gestione strategia ricevuta
	.
	.
	.
	*/
	//printf("PAWN SBLOCCA PLAYER\n");
					/* Unblock players */
    /* ----------------------------------------------------------------- */
	sem_reserve_1(player_sem_id, 0);
    /* ----------------------------------------------------------------- */
	//printf("PAWN HA SBLOCCATO PLAYER\n");


					/* Wait for 1 on synchro */
    /* ----------------------------------------------------------------- */
    if((master_sem_id = semget(MAIN_SEM, 3, 0666)) == -1){
    	printf("DIO CANE 2\n");
		if(errno == ENOENT)
			fprintf(stderr, "Failed access to memory for pawns\n");
		exit(EXIT_FAILURE);
	}
	//printf("DIO CANE 3\n");
	//printf("ASPETTO DI ESSERE SBLOCCATA: %d\n", semctl(master_sem_id, SYNCHRO,GETVAL));
	sem_reserve_1(master_sem_id, SYNCHRO);
	//printf("SONO STATA SBLOCCATA PAWN -_______-_-_-_--_____-----\n");
    /* ----------------------------------------------------------------- */


	printf("GAME INIZIATO PAWN\n");

	exit(EXIT_SUCCESS);

}
