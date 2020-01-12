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

typedef struct{
	int x;
	int y;
	int remaining_moves;
	int * directions;
} PAWN;

int has_moves(PAWN * pawn);
int move(int columns, int player_letter, PAWN * pawn, int * chessboard, int rows);
int move_specific(int way,int player_letter, PAWN * pawn, int * chessboard, int columns);
int fix_position(PAWN * pawn, int columns, int rows, int * chessboard, int player_letter);

int main(int argc, char *argv[]){

	int parameters_id;
	struct param * parameters;
	int player_sem_id;
	int chessboard_mem_id;
	int * chessboard;

	int player_msg_id;
	struct message message_to_pawn;
	struct player_strategy strategy;

	long type;
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

	PAWN pawn;

	parameters_id = atol(argv[1]);
	type = atol(argv[2]);
	player_letter = -atol(argv[3]);




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

	pawn.x = message_to_pawn.x;
	pawn.y = message_to_pawn.y;

	
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
    /*printf("STO PER LEGGERE LA STRATEGIA\n");*/
	

	if((a = msgrcv(player_msg_id, &strategy,LEN_STRATEGY, type, 0))== -1){
		fprintf(stderr, "MSGRCV: ret: %d, errno: %d, %s\n", a, errno, strerror(errno));
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

	/*
	.
	.
	. Gestione strategia ricevuta
	.
	.
	.
	*/
	
					/* Unblock players */
    /* ----------------------------------------------------------------- */
	sem_reserve_1(player_sem_id, 0);
    /* ----------------------------------------------------------------- */
	


					/* Wait for 1 on synchro */
    /* ----------------------------------------------------------------- */
    if((master_sem_id = semget(MAIN_SEM, 4, 0666)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed access to memory for pawns\n");
		exit(EXIT_FAILURE);
	}
	
	sem_reserve_1(master_sem_id, START);
    /* ----------------------------------------------------------------- */

	if(strategy.selected == 1){
		while(has_moves(&pawn)){

			if(semctl(chessboard_sem_id, flag_position, GETVAL) == 1){
				if(move(columns, player_letter, &pawn, chessboard, rows) == 2){
					if((a = msgrcv(player_msg_id, &strategy,LEN_STRATEGY, type, IPC_NOWAIT)) != -1){
						pawn.directions[N] = strategy.directions[N];
						pawn.directions[S] = strategy.directions[S];
						pawn.directions[E] = strategy.directions[E];
						pawn.directions[W] = strategy.directions[W];

						flag_position = strategy.flag_position;
						printf("PRESA IO %c: HO PRESO UN NUOVO MESSAGGIO (%d,%d) e ho (%d,%d,%d,%d)\n",-player_letter, pawn.x, pawn.y, pawn.directions[N], pawn.directions[S], pawn.directions[E], pawn.directions[W]);
					}else{
						break;
					}
				}	
			}else{
				if((a = msgrcv(player_msg_id, &strategy,LEN_STRATEGY, type, IPC_NOWAIT)) != -1){
					if(strategy.directions[N] > 0){
						if(pawn.directions[N] > 0){
							pawn.directions[N] += strategy.directions[N];
						}else{
							pawn.directions[N] = strategy.directions[N];
						}
					}else{
						pawn.directions[N] = 0;
					}

					if(strategy.directions[S] > 0){
						if(pawn.directions[S] > 0){
							pawn.directions[S] += strategy.directions[S];
						}else{
							pawn.directions[S] = strategy.directions[S];
						}
					}else{
						pawn.directions[S] = 0;
					}

					if(strategy.directions[E] > 0){
						if(pawn.directions[E] > 0){
							pawn.directions[E] += strategy.directions[E];
						}else{
							pawn.directions[E] = strategy.directions[E];
						}
					}else{
						pawn.directions[E] = 0;
					}

					if(strategy.directions[W] > 0){
						if(pawn.directions[W] > 0){
							pawn.directions[W] += strategy.directions[W];
						}else{
							pawn.directions[W] = strategy.directions[W];
						}
					}else{
						pawn.directions[W] = 0;
					}

					flag_position = strategy.flag_position;
					printf("PERDUTA %c: HO PRESO UN NUOVO MESSAGGIO (%d,%d) e ho (%d,%d,%d,%d)\n", -player_letter,pawn.x, pawn.y, pawn.directions[N], pawn.directions[S], pawn.directions[E], pawn.directions[W]);
				}else{
					break;
				}
			}
		}

	}
	free(pawn.directions);
	/* sem_reserve() => vai a dormire */
	

	/*
	.
	.
	.
	. GAME
	.
	.
	.
	*/

	exit(EXIT_SUCCESS);

}

int has_moves(PAWN * pawn){
	if(pawn->directions[N] > 0 || pawn->directions[S] > 0 || pawn->directions[E] > 0 || pawn->directions[W] > 0)
		return 1;
	else
		return 0;
}

int move(int columns, int player_letter, PAWN * pawn, int * chessboard, int rows, int player_letter){
	
	int result;
	result = -1;
	if(pawn->directions[N] > 0){
		result = move_specific(N, player_letter, pawn, chessboard, columns);
		if(result == 2){
			return 2;
		}else if(result == 1){
			return 1;
		}
	}
	if(pawn->directions[S] > 0){
		result = move_specific(S, player_letter, pawn, chessboard, columns);
		if(result == 2){
			return 2;
		}else if(result == 1){
			return 1;
		}
	}
	if(pawn->directions[E] > 0){
		result = move_specific(E, player_letter, pawn, chessboard, columns);
		if(result == 2){
			return 2;
		}else if(result == 1){
			return 1;
		}
	}
	if(pawn->directions[W] > 0){
		result = move_specific(W, player_letter, pawn, chessboard, columns);
		if(result == 2){
			return 2;
		}else if(result == 1){
			return 1;
		}
	}

	if(result == 0){
		printf("SONO BLOCCATA\n");
		return fix_position(pawn, columns, rows, chessboard, player_letter);
	}

	
	
	return 0;
}

int move_specific(int way,int player_letter, PAWN * pawn, int * chessboard, int columns){
	
	struct timespec timeout;
	int points;
	points = 0;
	timeout.tv_sec = 0;
	timeout.tv_nsec = 100000000;
	switch(way){
		case W:
			if(sem_reserve_1_time(chessboard_sem_id, pawn->y * columns + (pawn->x - 1)) != -1){
				/* Forse da fare controllo se ho preso una bandierina */
				pawn->x--;
				if(chessboard[pawn->y * columns + pawn->x] > 0){
					points = chessboard[pawn->y * columns + pawn->x];
					printf("%c : PRESA BANDIERA (%d,%d)\n", -player_letter, pawn->x , pawn->y);
				}
				chessboard[pawn->y * columns + pawn->x] = player_letter;
				if(player_letter == -65)
					chessboard[pawn->y * columns + (pawn->x + 1)] = -67;
				else
					chessboard[pawn->y * columns + (pawn->x + 1)] = -68;
				sem_release(chessboard_sem_id, pawn->y * columns + (pawn->x + 1));
				pawn->remaining_moves--;
				pawn->directions[W]--;
				/* NANOSLEEP */
				nanosleep(&timeout, NULL);
				if(points > 0){
					return 2;
				}else{
					return 1;
				}
			}else return 0;
		break;
		case E:
			if(sem_reserve_1_time(chessboard_sem_id, pawn->y * columns + (pawn->x + 1)) != -1){
				/* Forse da fare controllo se ho preso una bandierina */
				pawn->x++;

				if(chessboard[pawn->y * columns + pawn->x] > 0){
					points = chessboard[pawn->y * columns + pawn->x];
					printf("%c : PRESA BANDIERA (%d,%d)\n", -player_letter, pawn->x , pawn->y);
				}

				chessboard[pawn->y * columns + pawn->x] = player_letter;
				if(player_letter == -65)
					chessboard[pawn->y * columns + (pawn->x - 1)] = -67;
				else
					chessboard[pawn->y * columns + (pawn->x - 1)] = -68;
				sem_release(chessboard_sem_id, pawn->y * columns + (pawn->x - 1));
				pawn->remaining_moves--;
				pawn->directions[E]--;
				nanosleep(&timeout, NULL);
				if(points > 0){
					return 2;
				}else{
					return 1;
				}
				
			}else return 0;

		break;
		case S:
			if(sem_reserve_1_time(chessboard_sem_id, (pawn->y + 1) * columns + pawn->x) != -1){
				/* Forse da fare controllo se ho preso una bandierina */
				pawn->y++;

				if(chessboard[pawn->y * columns + pawn->x] > 0){
					points = chessboard[pawn->y * columns + pawn->x];
					printf("%c : PRESA BANDIERA (%d,%d)\n", -player_letter, pawn->x , pawn->y);
				}

				chessboard[pawn->y * columns + pawn->x] = player_letter;
				if(player_letter == -65)
					chessboard[(pawn->y - 1) * columns + pawn->x] = -67;
				else
					chessboard[(pawn->y - 1) * columns + pawn->x] = -68;
				sem_release(chessboard_sem_id, (pawn->y - 1) * columns + pawn->x);
				pawn->remaining_moves--;
				pawn->directions[S]--;
				nanosleep(&timeout, NULL);
				if(points > 0){
					return 2;
				}else{
					return 1;
				}
			}else return 0;

		break;
		case N:
			if(sem_reserve_1_time(chessboard_sem_id, (pawn->y - 1) * columns + pawn->x) != -1){
				/* Forse da fare controllo se ho preso una bandierina */
				pawn->y--;

				if(chessboard[pawn->y * columns + pawn->x] > 0){
					points = chessboard[pawn->y * columns + pawn->x];
					printf("%c : PRESA BANDIERA (%d,%d)\n", -player_letter, pawn->x , pawn->y);
				}

				chessboard[pawn->y * columns + pawn->x] = player_letter;
				if(player_letter == -65)
					chessboard[(pawn->y + 1) * columns + pawn->x] = -67;
				else
					chessboard[(pawn->y + 1) * columns + pawn->x] = -68;
				sem_release(chessboard_sem_id, (pawn->y + 1) * columns + pawn->x);
				pawn->remaining_moves--;
				pawn->directions[N]--;
				nanosleep(&timeout, NULL);
				if(points > 0){
					return 2;
				}else{
					return 1;
				}
			}else return 0;

		break;

		default:
			printf("FACCIO MERDA\n");
		break;
	}
}

int fix_position(PAWN * pawn, int columns, int rows, int * chessboard, int player_letter){
	if(pawn->directions[N] > 0){
		if(pawn->directions[E] > 0){
			/* controllo W */
			/* controllo S */
			if(pawn->x-1 >= 0 && semctl(chessboard_sem_id, pawn->y * columns + (pawn->x-1), GETVAL) == 1){
				if(pawn->y + 1 <= rows && semctl(chessboard_sem_id, (pawn->y+1) * columns + pawn->x, GETVAL) == 1){
					while(/* uno dei due non è occupato */){
						/* controllo N/W e S/E */
					}
				}else{
					return move_specific(W, player_letter, pawn, chessboard, columns);
				}
			}else{
				if(pawn->y + 1 <= rows && semctl(chessboard_sem_id, (pawn->y+1) * columns + pawn->x, GETVAL) == 1){
					return move_specific(S, player_letter, pawn, chessboard, columns);
				}else{
					/* QUA DEVI FERMARTI DIO FA => azzera le mosse disponibili */
					/*while( uno dei due non è libero ){
						 controllo N/W e S/E 
					}*/	
				}
			}
		}else if(pawn->directions[W] > 0){
			/* controllo E */
			/* controllo S */
			if(pawn->x+1 <= columns && semctl(chessboard_sem_id, pawn->y * columns + (pawn->x+1), GETVAL) == 1){
				if(pawn->y + 1 <= rows && semctl(chessboard_sem_id, (pawn->y+1) * columns + pawn->x, GETVAL) == 1){
					while(/* uno dei due non è occupato */){
						/* controllo N/E e S/W */
					}
				}else{
					return move_specific(E, player_letter, pawn, chessboard, columns);
				}
			}else{
				if(pawn->y + 1 <= rows && semctl(chessboard_sem_id, (pawn->y+1) * columns + pawn->x, GETVAL) == 1){
					return move_specific(S, player_letter, pawn, chessboard, columns);
				}else{
					while(/* uno dei due non è libero */){
						/* controllo N/E e S/W */
					}
				}
			}
		}else{
			/* controllo E */
			/* controllo W */
			if(pawn->x+1 <= columns && semctl(chessboard_sem_id, pawn->y * columns + (pawn->x+1), GETVAL) == 1){
				if(pawn->x-1 >= 0 && semctl(chessboard_sem_id, pawn->y * columns + (pawn->x-1), GETVAL) == 1){
					while(/* uno dei due non è occupato */){
						/* controllo N/E e N/W */
					}
				}else{
					return move_specific(E, player_letter, pawn, chessboard, columns);
				}
			}else{
				if(pawn->x-1 >= 0 && semctl(chessboard_sem_id, pawn->y * columns + (pawn->x-1), GETVAL) == 1){
					return move_specific(W, player_letter, pawn, chessboard, columns);
				}else{
					while(/* uno dei due non è libero */){
						/* controllo N/E e N/W */
					}
				}
			}

		}
		
	}

	if(pawn->directions[S] > 0){
		if(pawn->directions[E] > 0){
			/* controllo W */
			/* controllo N */
			if(pawn->x-1 >= 0 && semctl(chessboard_sem_id, pawn->y * columns + (pawn->x-1), GETVAL) == 1){
				if(pawn->y-1 >= 0 && semctl(chessboard_sem_id, (pawn->y-1) * columns + pawn->x, GETVAL) == 1){
					while(/* uno dei due non è occupato */){
						/* controllo N/E e S/W */
					}
				}else{
					return move_specific(W, player_letter, pawn, chessboard, columns);
				}
			}else{
				if(pawn->y-1 >= 0 && semctl(chessboard_sem_id, (pawn->y-1) * columns + pawn->x, GETVAL) == 1){
					return move_specific(N, player_letter, pawn, chessboard, columns);
				}else{
					while(/* uno dei due non è libero */){
						/* controllo N/E e S/W */
					}
				}
			}
		}else if(pawn->directions[W] > 0){
			/* controllo N */
			/* controllo E */
			if(pawn->x+1 <= columns && semctl(chessboard_sem_id, pawn->y * columns + (pawn->x+1), GETVAL) == 1){
				if(pawn->y-1 >= 0 && semctl(chessboard_sem_id, (pawn->y-1) * columns + pawn->x, GETVAL) == 1){
					while(/* uno dei due non è occupato */){
						/* controllo N/W e S/E */
					}
				}else{
					return move_specific(E, player_letter, pawn, chessboard, columns);
				}
			}else{
				if(pawn->y-1 >= 0 && semctl(chessboard_sem_id, (pawn->y-1) * columns + pawn->x, GETVAL) == 1){
					return move_specific(N, player_letter, pawn, chessboard, columns);
				}else{
					while(/* uno dei due non è libero */){
						/* controllo N/W e S/E */
					}
				}
			}
		}else{
			/* controllo E */
			/* controllo W */
			if(pawn->x+1 <= columns && semctl(chessboard_sem_id, pawn->y * columns + (pawn->x+1), GETVAL) == 1){
				if(pawn->x-1 >= 0 && semctl(chessboard_sem_id, pawn->y * columns + (pawn->x-1), GETVAL) == 1){
					while(/* uno dei due non è occupato */){
						/* controllo S/E e S/W */
					}
				}else{
					return move_specific(E, player_letter, pawn, chessboard, columns);
				}
			}else{
				if(pawn->x-1 >= 0 && semctl(chessboard_sem_id, pawn->y * columns + (pawn->x-1), GETVAL) == 1){
					move_specific(W, player_letter, pawn, chessboard, columns);
				}else{
					while(/* uno dei due non è libero */){
						/* controllo S/E e S/W */
					}
				}
			}

		}
	}

	if(pawn->directions[E] > 0){
		/* controllo N */
		/* controllo S */
		if(pawn->y-1 >= 0 && semctl(chessboard_sem_id, (pawn->y-1) * columns + pawn->x, GETVAL) == 1){
			if(pawn->y + 1 <= rows && semctl(chessboard_sem_id, (pawn->y+1) * columns + pawn->x, GETVAL) == 1){
				while(/* uno dei due non è occupato */){
					/* controllo N/E e S/E */
				}
			}else{
				return move_specific(N, player_letter, pawn, chessboard, columns);
			}
		}else{
			if(pawn->y + 1 <= rows && semctl(chessboard_sem_id, (pawn->y+1) * columns + pawn->x, GETVAL) == 1){
				return move_specific(S, player_letter, pawn, chessboard, columns);
			}else{
				while(/* uno dei due non è libero */){
					/* controllo N/E e S/E */
				}
			}
		}
	}
	if(pawn->directions[W] > 0){
		/* controllo N */
		/* controllo S */
		if(pawn->y-1 >= 0 && semctl(chessboard_sem_id, (pawn->y-1) * columns + pawn->x, GETVAL) == 1){
			if(pawn->y + 1 <= rows && semctl(chessboard_sem_id, (pawn->y+1) * columns + pawn->x, GETVAL) == 1){
				while(/* uno dei due non è occupato */){
					/* controllo N/W e S/W */
				}
			}else{
				return move_specific(N, player_letter, pawn, chessboard, columns);
			}
		}else{
			if(pawn->y + 1 <= rows && semctl(chessboard_sem_id, (pawn->y+1) * columns + pawn->x, GETVAL) == 1){
				return move_specific(S, player_letter, pawn, chessboard, columns);
			}else{
				while(/* uno dei due non è libero */){
					/* controllo N/W e S/W */
				}
			}
		}
	}
}
