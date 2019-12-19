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

void set_pawns(int letter, int parameters_id, int player_msg_id, int chessboard_mem_id, int chessboard_sem_id);

int main(int argc, char *argv[]){

	int i;
	int k;
	
	int parameters_id;
	struct param * parameters;
	
	pid_t select;

	int master_sem_id;
	int player_sem_id;

	int player_msg_id;
	char sprintf_type[40];
	int player_type;

	int turn_semaphore;

	int chessboard_sem_id;
	int chessboard_mem_id;
	int rows;
	int columns;


	int a;

	char * args[5]; 

	struct message message_to_pawn;
	struct strategy strategy_pawn;

	char sprintf_parameters_id[40];
	char sprintf_letter[40];

	char * strat;

	int test;

	int turn_sem_id;

	int turn_sem_entry;
	int j;
	struct position * positions;

	
				/* Checking passed arguments */
	/* ---------------------------------------------------------------- */
	if(argc < 2){
		fprintf(stderr, "Failed to pass parameter to players%s\n");
		exit(EXIT_FAILURE);
	}
	/* ---------------------------------------------------------------- */

	
	parameters_id = atoi(argv[1]);
	player_type = atoi(argv[2]);
	turn_sem_entry = atoi(argv[3]);

	/*printf("Player Tipe: %d, in char: %c\n", player_type,player_type);*/
	
	
	parameters = shmat(parameters_id,NULL,0);

	master_sem_id = semget(MAIN_SEM, 3, 0666);
    

	
    sprintf (sprintf_parameters_id, "%d", parameters_id);

    
    				/* Setting pawns parameters */
    /* -------------------------------------------------------- */
    args[0] = "./player";
    args[1] = sprintf_parameters_id;
    /* wait type on args[2]*/
    /* wait letter to pawn on args[3]*/
    args[4] = NULL;
    /* -------------------------------------------------------- */

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	rows = parameters->SO_ALTEZZA;
	columns = parameters->SO_BASE;

	chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(int) * rows * columns, 0666);
	chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, rows * columns, 0666);

				/* Getting the Turn Semaphore */
	/* --------------------------------------------------------------------- */
	turn_sem_id = semget(MUTUAL_TURN, parameters->SO_NUM_G, 0666);
	/* --------------------------------------------------------------------- */



			/* Access and set pawn semaphore to wait pawns*/
	/* -----------------------------------------------------------------*/
	if((player_sem_id = semget(getpid(), 1, 0666 | IPC_CREAT)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed semaphore for pawns\n");
		exit(EXIT_FAILURE);
	}
			
    sem_set_val(player_sem_id, 0, parameters->SO_NUM_P);
    /* -----------------------------------------------------------------*/

    /* ----- Setting message queue for players------ */
    player_msg_id = msgget(getpid(), 0666 | IPC_CREAT);


    					/* Critical section players */
    /* --------------------------------------------------------------------- */
    for(i = 0; i < parameters->SO_NUM_P; i++){
    	sem_reserve_1(turn_sem_id, (turn_sem_entry-1 + parameters->SO_NUM_G)% parameters->SO_NUM_G);
			set_pawns(player_type, parameters_id, player_msg_id, chessboard_mem_id, chessboard_sem_id);
			sprintf (sprintf_letter, "%d", player_type);
			args[3] = sprintf_letter;
		    /*printf("%c ", player_type);	
		    printf("CHI STO LIBERANDO: %d\n", turn_sem_entry);*/
	    sem_release(turn_sem_id, turn_sem_entry);
	}
    /* --------------------------------------------------------------------- */


    

    
    

    						/* Fork the pawns*/
    /* -------------------------------------------------------------------------- */
	for (i = 0; i < parameters->SO_NUM_P; i++){
		switch(fork()){
		case -1:
    		fprintf(stderr, "Failed to Fork Pawns PID#%d%s\n", getpid());
    		exit(EXIT_FAILURE);
    	case 0:
    		/* Giving the type */
    		sprintf (sprintf_type, "%d", i+1);
    		args[2] = sprintf_type;
    		if(execve("./pawn", args, NULL)){
    			fprintf(stderr, "Execve() failed #%d : %s\n", errno, strerror(errno));
    			exit(EXIT_FAILURE);
    		}
    	default:
    		break;
		}
	}
	/* -------------------------------------------------------------------------- */


						/* Wait for 0 for pawns*/
	/* -------------------------------------------------------------------------- */
    printf("Player aspetta %d\n", getpid());
    sem_reserve_0(player_sem_id, 0);
    printf("Player SINCRONIZZATO\n");
	/* -------------------------------------------------------------------------- */
    

    					/* Unblock master and wait on synchro */
	/* -------------------------------------------------------------------------- */
    sem_reserve_1(master_sem_id, MASTER);
    sem_reserve_1(master_sem_id, SYNCHRO);
	/* -------------------------------------------------------------------------- */

				/* Unblock pawns by send strategy and wait for them */
	/* -------------------------------------------------------------------------- */
	sem_set_val(player_sem_id, 0, parameters->SO_NUM_P);
	
	for(i=0; i < parameters->SO_NUM_P; i++){
		strategy_pawn.mtype = i+1;
		
		strncpy(strategy_pawn.strategy, "NNWENESS", 9);
		
		/*fprintf(stderr, "MSGSEND: ret: %d, errno: %d, %s\n", test, errno, strerror(errno));*/
		
		/*printf("STAMPO STRATEGY DA MANDARE: %s\n", strategy_pawn.strategy);*/

		test = msgsnd(player_msg_id, &strategy_pawn, sizeof(char) *  STRAT_LEN, 0);
		if(test == -1){
			fprintf(stderr, "MSGSEND: ret: %d, errno: %d, %s\n", test, errno, strerror(errno));
		}
	}
	
	sem_reserve_0(player_sem_id, 0);
	/* -------------------------------------------------------------------------- */


					/* Unblock master and wait for him */
	/* -------------------------------------------------------------------------- */
	sem_reserve_1(master_sem_id, A);
	sem_reserve_1(master_sem_id, MASTER);
	/* -------------------------------------------------------------------------- */


					/* Unblock pawns and START GAME */
	/* -------------------------------------------------------------------------- */
	sem_set_val(master_sem_id, SYNCHRO, parameters->SO_NUM_P * parameters->SO_NUM_G);
	/* -------------------------------------------------------------------------- */
       

    while((select = wait(NULL)) != -1);    
    
	exit(EXIT_SUCCESS);
}

/*
	Column constant = number of pawns per row / SO_BASE;
		Column (X) Step = First time = Column constant / 2;
				   		  Next times = Column constant;

	Row constant = number of pawns per column / SO_ALTEZZA;
		Row (Y) Step = First time = if even:
										(Row constant / 2) + 1;
									else:
										Row constant / 2
				   	   Next times = Row constant;
*/

void set_pawns(int letter, int parameters_id, int player_msg_id, int chessboard_mem_id, int chessboard_sem_id){
	static int k = 0;
	int i;
	int l;
	struct message message_to_pawn;
	int test;
	struct param * parameters;
	int positions_mem_id;
	struct position * positions;

	int wall;

	int * chessboard;
	int sem_chessboard;
	int j;
	int pawns_per_row;
	int pawns_per_column;
	

	parameters = shmat(parameters_id,NULL,0);

    chessboard = shmat(chessboard_mem_id,NULL,0);

    positions_mem_id = shmget(POSITION_MEM_KEY,sizeof(positions) * pawns_per_row * pawns_per_column, 0666 | IPC_CREAT);
    positions = (struct position *)shmat(positions_mem_id,NULL,0);

    
    
    	while(1){
    		i = rand() % parameters->SO_ALTEZZA;
    		j = rand() % parameters->SO_BASE;
    		
	    	test = sem_reserve_1_no_wait(chessboard_sem_id, positions[i * parameters->SO_BASE + j].y * parameters->SO_BASE + positions[i * parameters->SO_BASE + j].x);
	    	if(test != -1 && errno == EAGAIN){
	    		message_to_pawn.mtype = k+1;
	    		message_to_pawn.y = positions[i * parameters->SO_BASE + j].y;
				message_to_pawn.x = positions[i * parameters->SO_BASE + j].x;;
				break;
	    	}
	    }
	    		
	   k++;
	    
	
	/*printf("Type Pedina: %d\n", k);*/
	
    /*if(letter == 69)
    	/*printf("DEFAULT----------------------\n");

	printf("5\n");
	if(letter == 65){
		i = 0;
		l = 0;
		printf("3A\n");
	}else if(letter == 66){
		i = 0;
		l = parameters->SO_BASE-1;
		printf("3B\n");
	}else if(letter == 67){
		i = 0;
		l = (parameters->SO_BASE/4)-1;
		
		wall = l + l + l;
		printf("L per la C vale:     ----  --  letter:%d\n", letter);
	}else if(letter == 68){
		i = parameters->SO_ALTEZZA-1;
		l = (parameters->SO_BASE/4)-1;
		
		wall = l + l + l;
		/*printf("L per la D vale:     ----  --  letter:%d\n", letter);
	}else{
		/*printf("-----------------DEFAULT----------------------\n");
	}
	
	for(k = 0; k < parameters->SO_NUM_P; k++){
    	
		message_to_pawn.mtype = k+1;
		message_to_pawn.y = i;
		message_to_pawn.x = l;

		switch(letter){
			case 65:
			case 66:
				i++;
				if(i == parameters->SO_ALTEZZA){
					i = 0;
					if(letter == 65){
				    	l++;
					}else{
					    l--;
					}
				}

			break;

			case 67:
			case 68:
				l++;
				if(l == wall){
					l = (parameters->SO_BASE/4)-1;
					if(letter == 67){
						i++;
					}else{
						i--;
					}
				}

			break;

			default:
				/*while(1){
					message_to_pawn.y = rand() % parameters->SO_ALTEZZA;
					message_to_pawn.x = rand() % parameters->SO_BASE;
					if(chessboard[message_to_pawn.y * parameters->SO_BASE + message_to_pawn.x] == 1)
						break;
				}
				message_to_pawn.y = 0;
				message_to_pawn.x = 0;
			break;

		}*/
		/*printf("STO PER INSERIRE NELLA MATRICE: (%d, %d) => %d\n",message_to_pawn.y, message_to_pawn.x, message_to_pawn.y * parameters->SO_BASE + message_to_pawn.x  );*/
		chessboard[message_to_pawn.y * parameters->SO_BASE + message_to_pawn.x] = -letter;
		
		/*semctl(chessboard_sem_id, message_to_pawn.y * parameters->SO_BASE + message_to_pawn.x, SETVAL, 0);*/


			    /*message_to_pawn.strategy = "NSWEEWSN"*/
		/*printf("STO PER INVIARE\n");*/
		test = msgsnd(player_msg_id, &message_to_pawn, sizeof(int) * 2, 0);
		if(test == -1){
			fprintf(stderr, "MSGSEND: ret: %d, errno: %d, %s\n", test, errno, strerror(errno));
		}	    
       
    
}