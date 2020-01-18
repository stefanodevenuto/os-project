#define _GNU_SOURCE
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
#include <sys/time.h>
#include "semaphore.h"

int chessboard_sem_id;
long type;
int player_msg_id_results;
struct param * parameters;

typedef struct{
	int x;
	int y;
	int remaining_moves;
	int * directions;
} PAWN;

int has_moves(PAWN * pawn);
int move(int columns, int player_letter, PAWN * pawn, int * chessboard, int rows, int player_msg_id_results, int type, int * taken);
int move_specific(int way,int player_letter, PAWN * pawn, int * chessboard, int columns, int player_msg_id_results, int type, int * taken);
int fix_position(PAWN * pawn, int columns, int rows, int * chessboard, int player_letter);

int main(int argc, char *argv[]){

	

	int parameters_id;
	
	int player_sem_id;
	int chessboard_mem_id;
	int * chessboard;

	int player_msg_id;
	struct message message_to_pawn;
	struct player_strategy strategy;

	
	int player_letter;
	int a;
	int master_sem_id;
	int * directions;
	int flag_x;
	int flag_y;
	int flag_position;

	int rows;
	int columns;
	int i;
	int taken;

	PAWN pawn;

	
	struct pawn_flag to_player;
	int flag;

	struct end_round_message end_round_to_players;


	parameters_id = atol(argv[1]);
	type = atol(argv[2]);
	player_letter = -atol(argv[3]);
	player_msg_id_results = atol(argv[4]);


		  /* Getting chessboard, player semaphore and player queue*/
	/* ---------------------------------------------------------------- */
	parameters = shmat(parameters_id,NULL,0);
	player_sem_id = semget(getppid(), 4, 0666);
	player_msg_id = msgget(getppid(), 0666);
	/* ---------------------------------------------------------------- */

	rows = parameters->SO_ALTEZZA;
	columns = parameters->SO_BASE;

		     /* Receiving coordinates and accessing chessboard*/
	/* ---------------------------------------------------------------- */
	a = msgrcv(player_msg_id, &message_to_pawn, LEN_X_Y, type, 0);
	if(a == -1){
		fprintf(stderr, "MSGRCV: ret: %d, errno: %d, %s\n", a, errno, strerror(errno));
	}

	pawn.x = message_to_pawn.x;
	pawn.y = message_to_pawn.y;
	pawn.remaining_moves = parameters->SO_N_MOVES;

	
	if((chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(int) * rows * columns, 0666)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed access to memory for pawns\n");
		exit(EXIT_FAILURE);
	}
	chessboard = shmat(chessboard_mem_id,NULL,0);

	chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, rows * columns, 0666);
	/* ---------------------------------------------------------------- */

					/* Unblock players */
    /* ----------------------------------------------------------------- */
	sem_reserve_1(player_sem_id, 0);
    /* ----------------------------------------------------------------- */


					/* Wait for strategy */
    /* ----------------------------------------------------------------- */
	
	while(1){
		/* Waiting on the Start-Round semaphore */
		sem_reserve_1(player_sem_id, 3);
		if((a = msgrcv(player_msg_id, &strategy,LEN_STRATEGY, type, 0))== -1){
			fprintf(stderr, "MSGRCV PAWN: ret: %d, errno: %d, %s\n", a, errno, strerror(errno));
		}

		/*printf("Pedina riceve strategia: %s\n", strategy_pawn.strategy);*/
	    /* ----------------------------------------------------------------- */

		pawn.directions = (int *)malloc(sizeof(int) * 4);



		if(strategy.selected == 1){

			pawn.directions[N] = strategy.directions[N];
			pawn.directions[S] = strategy.directions[S];
			pawn.directions[E] = strategy.directions[E];
			pawn.directions[W] = strategy.directions[W];

			flag_x = strategy.flag_x;
			flag_y = strategy.flag_y;
			flag_position = strategy.flag_position;

		}

		

						/* Unblock players */
	    /* ----------------------------------------------------------------- */
		sem_reserve_1(player_sem_id, 0);
	    /* ----------------------------------------------------------------- */
		


						/* Wait for 1 on START */
	    /* ----------------------------------------------------------------- */
	    if((master_sem_id = semget(MAIN_SEM, 5, 0666)) == -1){
			if(errno == ENOENT)
				fprintf(stderr, "Failed access to memory for pawns\n");
			exit(EXIT_FAILURE);
		}
		
		sem_reserve_1(master_sem_id, START);
	    /* ----------------------------------------------------------------- */
		taken = 0;
		if(strategy.selected == 1){
			flag = 0;
			while(1){
				while(has_moves(&pawn)){
					if(chessboard[flag_position] > 0){
						move(columns, player_letter, &pawn, chessboard, rows, player_msg_id_results,type,&taken);
					}else{
						break;
					}
				}
				if((a = msgrcv(player_msg_id, &strategy,LEN_STRATEGY, type, IPC_NOWAIT)) != -1){
					/* Re-Calculate the new dispositions of the Player */
					pawn.directions[N] += strategy.directions[N];
					pawn.directions[S] += strategy.directions[S];
					pawn.directions[E] += strategy.directions[E];
					pawn.directions[W] += strategy.directions[W];
					flag_position = strategy.flag_position;

					if(pawn.directions[N] > 0 && pawn.directions[S] > 0){
						if(pawn.directions[N] > pawn.directions[S]){
							pawn.directions[N] -= pawn.directions[S];
							pawn.directions[S] = 0;
						}else{
							pawn.directions[S] -= pawn.directions[N];
							pawn.directions[N] = 0;
						}
					}

					if(pawn.directions[W] > 0 && pawn.directions[E] > 0){
						if(pawn.directions[W] > pawn.directions[E]){
							pawn.directions[W] -= pawn.directions[E];
							pawn.directions[E] = 0;
						}else{
							pawn.directions[E] -= pawn.directions[W];
							pawn.directions[W] = 0;
						}
					}
				}else break;
			}	
		}
		/* Unblock Player */
		sem_reserve_1(player_sem_id, 1);

		/* Wait the sending of the second half of messages */
		sem_reserve_1(player_sem_id, 2);

		end_round_to_players.mtype = type;
		end_round_to_players.x = pawn.x;
		end_round_to_players.y = pawn.y;
		end_round_to_players.remaining_moves = pawn.remaining_moves;

		if(msgsnd(player_msg_id, &end_round_to_players, END_ROUND_MESSAGE, 0) == -1){
			fprintf(stderr, "Failed Message Send Presa#%d: %s\n", errno, strerror(errno));
		}else{

		}

		free(pawn.directions);
	}
}

int has_moves(PAWN * pawn){
	if(pawn->directions[N] > 0 || pawn->directions[S] > 0 || pawn->directions[E] > 0 || pawn->directions[W] > 0)
		return 1;
	else
		return 0;
}

int move(int columns, int player_letter, PAWN * pawn, int * chessboard, int rows, int player_msg_id_results, int type, int * taken){
	
	int result;
	struct pawn_flag to_player;
	result = -1;
	if(pawn->directions[N] > 0){
		result = move_specific(N, player_letter, pawn, chessboard, columns, player_msg_id_results, type, taken);
		if(result == 2){
			return 2;
		}else if(result == 1){
			return 1;
		}
	}
	if(pawn->directions[S] > 0){
		result = move_specific(S, player_letter, pawn, chessboard, columns, player_msg_id_results, type, taken);
		if(result == 2){
			return 2;
		}else if(result == 1){
			return 1;
		}
	}
	if(pawn->directions[E] > 0){
		result = move_specific(E, player_letter, pawn, chessboard, columns, player_msg_id_results, type, taken);
		if(result == 2){
			return 2;
		}else if(result == 1){
			return 1;
		}
	}
	if(pawn->directions[W] > 0){
		result = move_specific(W, player_letter, pawn, chessboard, columns, player_msg_id_results, type, taken);
		if(result == 2){
			return 2;
		}else if(result == 1){
			return 1;
		}
	}

	pawn->directions[N] = 0;
	pawn->directions[S] = 0;
	pawn->directions[E] = 0;
	pawn->directions[W] = 0;

	return 0;
}

int move_specific(int way,int player_letter, PAWN * pawn, int * chessboard, int columns, int player_msg_id_results, int type, int * taken){
	
	struct timespec timeout;
	struct pawn_flag to_player;
	int flag;
	flag = 0;
	
	timeout.tv_sec = 0;
	timeout.tv_nsec = parameters->SO_MIN_HOLD_NSEC;
	switch(way){
		case W:
			if(sem_reserve_1_time(chessboard_sem_id, pawn->y * columns + (pawn->x - 1)) != -1){
				pawn->x--;
				if(chessboard[pawn->y * columns + pawn->x] > 0){
					to_player.points = chessboard[pawn->y * columns + pawn->x];
					to_player.mtype = type;
					flag = 1;
					*taken = 1;
					if(msgsnd(player_msg_id_results, &to_player, TO_PLAYER, 0) == -1){
					   	fprintf(stderr, "Failed Message Send Presa#%d: %s\n", errno, strerror(errno));
					}
					printf("%c : FLAG TAKEN (%d,%d)\n", -player_letter, pawn->x , pawn->y);
				}
				chessboard[pawn->y * columns + pawn->x] = player_letter;
				chessboard[pawn->y * columns + (pawn->x + 1)] = 0;
				sem_release(chessboard_sem_id, pawn->y * columns + (pawn->x + 1));
				pawn->remaining_moves--;
				pawn->directions[W]--;
				
				nanosleep(&timeout, NULL);
				if(flag > 0){
					return 2;
				}else{
					return 1;
				}
			}else return 0;
		break;
		case E:
			if(sem_reserve_1_time(chessboard_sem_id, pawn->y * columns + (pawn->x + 1)) != -1){
				pawn->x++;

				if(chessboard[pawn->y * columns + pawn->x] > 0){
					to_player.points = chessboard[pawn->y * columns + pawn->x];
					to_player.mtype = type;
					flag = 1;
					*taken = 1;
					if(msgsnd(player_msg_id_results, &to_player, TO_PLAYER, 0) == -1){
					   	fprintf(stderr, "Failed Message Send Presa#%d: %s\n", errno, strerror(errno));
					}
					printf("%c : FLAG TAKEN (%d,%d)\n", -player_letter, pawn->x , pawn->y);
				}

				chessboard[pawn->y * columns + pawn->x] = player_letter;
				chessboard[pawn->y * columns + (pawn->x - 1)] = 0;
				sem_release(chessboard_sem_id, pawn->y * columns + (pawn->x - 1));
				pawn->remaining_moves--;
				pawn->directions[E]--;
				nanosleep(&timeout, NULL);
				if(flag > 0){
					return 2;
				}else{
					return 1;
				}
				
			}else return 0;

		break;
		case S:
			if(sem_reserve_1_time(chessboard_sem_id, (pawn->y + 1) * columns + pawn->x) != -1){
				pawn->y++;

				if(chessboard[pawn->y * columns + pawn->x] > 0){
					to_player.points = chessboard[pawn->y * columns + pawn->x];
					to_player.mtype = type;
					flag = 1;
					*taken = 1;
					if(msgsnd(player_msg_id_results, &to_player, TO_PLAYER, 0) == -1){
					   	fprintf(stderr, "Failed Message Send Presa#%d: %s\n", errno, strerror(errno));
					}
					printf("%c : FLAG TAKEN (%d,%d)\n", -player_letter, pawn->x , pawn->y);
				}

				chessboard[pawn->y * columns + pawn->x] = player_letter;
				chessboard[(pawn->y - 1) * columns + pawn->x] = 0;
				sem_release(chessboard_sem_id, (pawn->y - 1) * columns + pawn->x);
				pawn->remaining_moves--;
				pawn->directions[S]--;
				nanosleep(&timeout, NULL);
				if(flag > 0){
					return 2;
				}else{
					return 1;
				}
			}else return 0;

		break;
		case N:
			if(sem_reserve_1_time(chessboard_sem_id, (pawn->y - 1) * columns + pawn->x) != -1){
				pawn->y--;

				if(chessboard[pawn->y * columns + pawn->x] > 0){
					to_player.points = chessboard[pawn->y * columns + pawn->x];
					to_player.mtype = type;
					flag = 1;
					*taken = 1;
					if(msgsnd(player_msg_id_results, &to_player, TO_PLAYER, 0) == -1){
					   	fprintf(stderr, "Failed Message Send Presa#%d: %s\n", errno, strerror(errno));
					}
					printf("%c : FLAG TAKEN (%d,%d)\n", -player_letter, pawn->x , pawn->y);
				}

				chessboard[pawn->y * columns + pawn->x] = player_letter;
				chessboard[(pawn->y + 1) * columns + pawn->x] = 0;
				sem_release(chessboard_sem_id, (pawn->y + 1) * columns + pawn->x);
				pawn->remaining_moves--;
				pawn->directions[N]--;
				nanosleep(&timeout, NULL);
				if(flag > 0){
					return 2;
				}else{
					return 1;
				}
			}else return 0;

		break;

		default:
			printf("Error on Moves-Switch\n");
		break;
	}
}

