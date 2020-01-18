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
#include <limits.h>
#include <math.h>
#include "semaphore.h"

struct param * parameters;
int pawns_number;
int player_sem_id;
int player_msg_id;
int player_msg_id_results;

struct flag{
	int number;
	int position;
	int x;
	int y;
	int distance;
	int checked;
};

struct pawn{
    long type;
    int pid;
    int x;
    int y;
    int starting_x;
    int starting_y;
    int remaining_moves;
    struct flag * target;
    struct flag * temp_target;
    int assigned;
    int temp_assigned;
};

struct pawn * pawns;

int set_pawns(int letter, int parameters_id, int player_msg_id, int chessboard_mem_id, int chessboard_sem_id, int rows, int columns, struct pawn * pawns);
struct position * calculate_position(int parameters_id);
int all_checked_flag(struct flag * flags, int flags_number);
void sigint_handler (int signal);

int main(int argc, char *argv[]){

	struct sigaction sa;

	int i;
	int k;
	
	int parameters_id;
	
	
	pid_t select;

	int master_sem_id;
	

	
	char sprintf_type[200];
	int player_type;

	int turn_semaphore;

	int chessboard_sem_id;
	int chessboard_mem_id;
	int rows;
	int columns;

	int a;

	char * args[5]; 

	struct message message_to_pawn;
	

	char sprintf_parameters_id[200];
	char sprintf_letter[200];
	char sprintf_player_msg_id_results[200];

	char * strat;

	int test;

	int turn_sem_id;

	int turn_sem_entry;
	int j;
	struct position * positions;
	int positions_id;

	

	struct flag * flags;
	struct flag * copy_flags;

	int * chessboard;
	int flags_number;
	unsigned int size;

	int flag_column;
	int flag_row;

	int distance;
	int min_distance;
	int pawn_row;
	int pawn_column;
	int new_pawn_column;
	int new_pawn_row;
	int target_index;
	int pos;
	int new_pos;
	int position;

	int target_count;
	int flags_index;

	int distance_2;
	int tmp;

	int temp_target_count;
	int flag_target;

	int index_best_distance;
	int pawn_index;
	int number;
	int def_distance;
	int def_distance_2;
	int test_pawn;
	int success;

	struct player_strategy strategy;

	

	int to_receive_targets;
	struct pawn_flag to_player;
	int total_points;

	int master_msg_id;
	struct pawn_flag to_master;

	int wait_pawns_player_sem_id;

	struct end_round_message end_round_from_pawn;

	int total_used_moves;
	int total_remaining_moves;
	struct pawn_flag moves_to_master;

	bzero(&sa, sizeof(sa));

    sa.sa_handler = sigint_handler;

    sigaction(SIGINT, &sa, NULL);


	
	parameters_id = atoi(argv[1]);
	player_type = atoi(argv[2]);
	turn_sem_entry = atoi(argv[3]);

	/*printf("Player Tipe: %d, in char: %c\n", player_type,player_type);*/
	
	
	parameters = shmat(parameters_id,NULL,0);

	master_sem_id = semget(MAIN_SEM, 5, 0666);
    
    total_points = 0;
    pawns_number = parameters->SO_NUM_P;
	
    sprintf (sprintf_parameters_id, "%d", parameters_id);
    

    
    				/* Setting pawns parameters */
    /* -------------------------------------------------------- */
    args[0] = "./player";
    args[1] = sprintf_parameters_id;
    /* wait type on args[2]*/
    /* wait letter to pawn on args[3]*/
    args[4] = NULL;
    args[5] = NULL;
    /* -------------------------------------------------------- */

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	rows = parameters->SO_ALTEZZA;
	columns = parameters->SO_BASE;

	chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(int) * rows * columns, 0666);
	chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, rows * columns, 0666);

	chessboard = shmat(chessboard_mem_id,NULL,0);

				/* Getting the Turn Semaphore */
	/* --------------------------------------------------------------------- */
	turn_sem_id = semget(MUTUAL_TURN, parameters->SO_NUM_G, 0666);
	/* --------------------------------------------------------------------- */



			/* Access and set pawn semaphore to wait pawns*/
	/* -----------------------------------------------------------------*/
	if((player_sem_id = semget(getpid(), 4, 0666 | IPC_CREAT)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed semaphore for pawns\n");
		exit(EXIT_FAILURE);
	}
			
    sem_set_val(player_sem_id, 0, parameters->SO_NUM_P);
    /* -----------------------------------------------------------------*/

    /* ----- Setting message queues for players------ */
    player_msg_id = msgget(getpid(), 0666 | IPC_CREAT);
    player_msg_id_results = msgget(IPC_PRIVATE, 0666);
    master_msg_id = msgget(getppid(), 0666);


    pawns = malloc(sizeof(struct pawn) * parameters->SO_NUM_P);
    

    					/* Critical section players */
    
    /* --------------------------------------------------------------------- */
    printf("Player %c : Positioning pawns...\n", player_type);
    for(i = 0; i < parameters->SO_NUM_P; i++){
    	sem_reserve_1(turn_sem_id, (turn_sem_entry-1 + parameters->SO_NUM_G)% parameters->SO_NUM_G);
			position = set_pawns(player_type, parameters_id, player_msg_id, chessboard_mem_id, chessboard_sem_id, rows, columns, pawns);
	    sem_release(turn_sem_id, turn_sem_entry);

	    pawns[i].remaining_moves = parameters->SO_N_MOVES;
	    pawns[i].starting_x = pawns[i].x = position % columns;
	    pawns[i].starting_y = pawns[i].y = (position - pawns[i].x) / columns;
	    pawns[i].assigned = 0;
	    
	}
    /* --------------------------------------------------------------------- */

	

    sprintf (sprintf_letter, "%d", player_type);
	args[3] = sprintf_letter;


    

    
    
    						/* Fork the pawns*/
    /* -------------------------------------------------------------------------- */
	for (i = 0; i < parameters->SO_NUM_P; i++){
		switch(pawns[i].pid = fork()){
		case -1:
    		fprintf(stderr, "Failed to Fork Pawns PID#%d%s\n", getpid());
    		exit(EXIT_FAILURE);
    		break;
    	case 0:
    		/* Giving the type */
    		sprintf (sprintf_type, "%d", i+1);
    		args[2] = sprintf_type;
    		sprintf (sprintf_player_msg_id_results, "%d", player_msg_id_results);
    		args[4] = sprintf_player_msg_id_results;
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
    sem_reserve_0(player_sem_id, 0);

    sem_reserve_1(master_sem_id, MASTER);
	/* -------------------------------------------------------------------------- */
    
    while(1){
	    					/* Unblock master and wait on synchro */
		/* -------------------------------------------------------------------------- */
	    
	    sem_reserve_1(master_sem_id, SYNCHRO);    

		/* -------------------------------------------------------------------------- */

					/* Unblock pawns by send strategy and wait for them */
		/* -------------------------------------------------------------------------- */
	    target_count = 0;
	    to_receive_targets = 0;
	    

	    flags = malloc(sizeof(struct flag));
	    flags_number = 0;
	    for(i = 0; i < rows; i++){
	    	for(j = 0; j < columns; j++){
	    		if(chessboard[i * columns + j] > 0){
	    			flags[flags_number].number = flags_number;
	    			flags[flags_number].position = i * columns + j;
	    			flags[flags_number].x = flags[flags_number].position % columns;
	    			flags[flags_number].y = (flags[flags_number].position - flags[flags_number].x) / columns;
	    			flags_number++;

	    			flags = realloc(flags, sizeof(struct flag) * (flags_number + 1));
	    			
	    		}
	    	}
	    }

	    for(i = 0; i < flags_number; i++ ){
	    	flags[i].checked = 0;
	    }

	    

	    for(i = 0; i < parameters->SO_NUM_P; i++){
			pawns[i].target = (struct flag *) malloc(sizeof(struct flag) * flags_number);
			memset(pawns[i].target, -1, sizeof(struct flag) * flags_number);
			pawns[i].temp_target = (struct flag *) malloc(sizeof(struct flag) * flags_number);
			memset(pawns[i].temp_target, -1, sizeof(struct flag) * flags_number);
	    }


	   

	    /* Setting the wait-for-0 semaphore */
	    sem_set_val(player_sem_id, 0, parameters->SO_NUM_P);
	    

	    /*---------------------------- ALGORITHM --------------------------------------*/
	    /*The aim of this Algorithm is to split the calculations in two parts: 
	      1. Calculate the nearest pawn for every flag and associate the flag in the 
	         temp_target array of the pawn itself
	      2. Until every flag is checked, the second part takes all the assigned pawns and 
	         re-order the flags by their distances*/

		    while(!all_checked_flag(flags,flags_number)){	
		    	/* PRIMA PARTE */
		    	strategy.selected = 0;
			    for(i = 0; i < flags_number; i++){
				    if(flags[i].checked != 1){
				    	target_index = -1;
				    	min_distance = INT_MAX;
					    temp_target_count = 0;
				    	
					    flag_column = flags[i].x;
					    flag_row = flags[i].y;
				    	
				    	test_pawn = 0;
						for(j = 0; j < parameters->SO_NUM_P; j++){

						    pawn_column = pawns[j].x;
						    pawn_row = pawns[j].y;

						    distance = abs(pawn_column - flag_column) + abs(pawn_row - flag_row);

						    if(pawns[j].remaining_moves >= distance && distance <= min_distance){
						    	min_distance = distance;
						    	target_index = j;	
						    }
					   	}
					    	
					    if(min_distance != INT_MAX){
					    	test_pawn = 1;
					    	temp_target_count = pawns[target_index].temp_assigned;
						    pawns[target_index].temp_target[temp_target_count] = flags[i];
						    pawns[target_index].temp_assigned += 1;
					    }


					    if(target_index == -1){
					    	flags[i].checked = 1;
						}
					}
			    }

			    /* SECONDA PARTE */
			    for(i = 0; i < parameters->SO_NUM_P; i++){
			    	target_index = 0;

			    	if(pawns[i].temp_assigned > 0){

			    		flag_column = pawns[i].temp_target[0].x;
			    		
				    	flag_row = pawns[i].temp_target[0].y;
				    	
				    	def_distance = abs(pawns[i].x - flag_column) + abs(pawns[i].y - flag_row);

				    	if(pawns[i].temp_assigned > 1){
				    		
				    		for(j = 1; j < pawns[i].temp_assigned; j++){	    			
				    			flag_column = pawns[i].temp_target[j].x;
				    			flag_row = pawns[i].temp_target[j].y;
				    			def_distance_2 = abs(pawns[i].x - flag_column) + abs(pawns[i].y - flag_row);

				    			if(def_distance_2 < def_distance){
				    				def_distance = def_distance_2;
				    				target_index = j;	
				    			}

				    		}
				    	}

				    	pawn_index = pawns[i].assigned;
				    	
				    	pawns[i].target[pawn_index] = pawns[i].temp_target[target_index];
				    	number = pawns[i].temp_target[target_index].number;
				    	flags[number].checked = 1;
				    			    	
				    	pawns[i].remaining_moves -= def_distance;

				    	strategy.mtype = pawns[i].type;
				    	strategy.selected = 1;
				    	strategy.flag_x = pawns[i].target[pawn_index].x;
				    	strategy.flag_y = pawns[i].target[pawn_index].y;
				    	strategy.flag_position = strategy.flag_y * columns + strategy.flag_x;

				    	strategy.directions[N] = 0;
				    	strategy.directions[S] = 0;
				    	strategy.directions[E] = 0;
				    	strategy.directions[W] = 0;


				    	if(pawns[i].temp_target[target_index].x > pawns[i].x){
				    		strategy.directions[E] = pawns[i].temp_target[target_index].x - pawns[i].x;
				    	}else if(pawns[i].temp_target[target_index].x < pawns[i].x){
				    		strategy.directions[W] = pawns[i].x - pawns[i].temp_target[target_index].x;
				    	}

				    	if(pawns[i].temp_target[target_index].y < pawns[i].y){
				    		strategy.directions[N] = pawns[i].y - pawns[i].temp_target[target_index].y;
				    	}else if(pawns[i].temp_target[target_index].y > pawns[i].y){
				    		strategy.directions[S] = pawns[i].temp_target[target_index].y - pawns[i].y;
				    	}


				    	pawns[i].x = pawns[i].target[pawn_index].x;
				    	pawns[i].y = pawns[i].target[pawn_index].y;
				    	
				    	pawns[i].assigned += 1;

				    	to_receive_targets++;

						if((success = msgsnd(player_msg_id, &strategy, LEN_STRATEGY, 0)) == -1){
						   	fprintf(stderr, "Failed Message Send#%d: %s\n", errno, strerror(errno));
						}

				    	for(j = 0; j < pawns[i].temp_assigned; j++){
				    		pawns[i].temp_assigned = 0;
				    	}
			    	}
			    }
			}

			for(i = 0; i < parameters->SO_NUM_P; i++){
				if(pawns[i].assigned == 0){
					strategy.mtype = pawns[i].type;
				    strategy.selected = 0;
				    strategy.directions[N] = 0;
				    strategy.directions[S] = 0;
				    strategy.directions[E] = 0;
				    strategy.directions[W] = 0;
				    strategy.flag_x = -1;
				    strategy.flag_y = -1;
				    strategy.flag_position = -1;

				    if((success = msgsnd(player_msg_id, &strategy, LEN_STRATEGY, 0)) == -1){
						fprintf(stderr, "Failed Message Send#%d: %s\n", errno, strerror(errno));
					}
				}
			}

			free(flags);
	    /*-----------------------------------------------------------------------------------*/
		

		/* Print of the Player Assignements for the selected Pawns */
		for(i = 0; i < parameters->SO_NUM_P; i++){
	    	if(pawns[i].assigned > 0){
		    	printf("\nPlayer: %c-------------------------- PAWN %d (%d,%d) ------------------------\n",player_type, pawns[i].type,pawns[i].starting_x,pawns[i].starting_y );	
		    	for(j = 0; j < flags_number; j++){
		    		if(pawns[i].target[j].x > -1)
		    			printf("Player: %c %d: Target #%d => (%d,%d) Moves: %d\n",player_type, i, j,pawns[i].target[j].x,pawns[i].target[j].y ,pawns[i].remaining_moves);
		    		else
		    			break;
		    	}
	    	}
	    }

		sem_set_val(player_sem_id, 3, parameters->SO_NUM_P);
		
		
				/* Wait-for-0 the reception of the Strategy by the Pawns*/
		/* -------------------------------------------------------------------------- */
		sem_reserve_0(player_sem_id, 0);
		/* -------------------------------------------------------------------------- */

					/* Set entry 1 to wait the flag messages */
		/* -------------------------------------------------------------------------- */
		sem_set_val(player_sem_id, 1, parameters->SO_NUM_P);
		/* -------------------------------------------------------------------------- */

						/* Unblock master and wait for him */
		/* -------------------------------------------------------------------------- */
		sem_reserve_1(master_sem_id, A);
		sem_reserve_1(master_sem_id, MASTER);
		/* -------------------------------------------------------------------------- */


						/* Unblock pawns and START GAME */
		/* -------------------------------------------------------------------------- */
		sem_set_val(master_sem_id, START, parameters->SO_NUM_P * parameters->SO_NUM_G);
		/* -------------------------------------------------------------------------- */


/* --------------------------------------- START TIME -------------------------------------------------------*/


						/* Wait for 0 on 1 */
		/* -------------------------------------------------------------------------- */
		sem_reserve_0(player_sem_id, 1);
		/* -------------------------------------------------------------------------- */


						/* Read taken flag messages*/
		/* -------------------------------------------------------------------------- */
		while((a = msgrcv(player_msg_id_results, &to_player,TO_PLAYER, 0, IPC_NOWAIT)) != -1){
			total_points += to_player.points;
			to_master.mtype = player_type;
			to_master.points = to_player.points;
			if(msgsnd(master_msg_id, &to_master, TO_PLAYER, 0) == -1){
				fprintf(stderr, "Failed Message Send PUNTEGGIO#%d: %s\n", errno, strerror(errno));
			}
		}
		/* -------------------------------------------------------------------------- */


/* --------------------------------------- END TIME -------------------------------------------------------*/


		/* Unblock pawns to send the remaining parameters to the players*/
		sem_set_val(player_sem_id,2,parameters->SO_NUM_P);


		total_remaining_moves = 0;

		for(i = 0; i < parameters->SO_NUM_P; i++){
			if((a = msgrcv(player_msg_id, &end_round_from_pawn,END_ROUND_MESSAGE, 0, 0)) != -1){
				for(j = 0; j < parameters->SO_NUM_P; j++){
					if(pawns[j].type == end_round_from_pawn.mtype){
						pawns[j].x = end_round_from_pawn.x;
						pawns[j].y = end_round_from_pawn.y;
						pawns[j].remaining_moves = end_round_from_pawn.remaining_moves;
						pawns[j].starting_x = end_round_from_pawn.x;
						pawns[j].starting_y = end_round_from_pawn.y;
						total_remaining_moves += end_round_from_pawn.remaining_moves;
					}
				}
			}else{
				fprintf(stderr, "Error Receive Message PLAYER\n");
			}
		}


	    total_used_moves = (parameters->SO_N_MOVES * parameters->SO_NUM_P) - total_remaining_moves;


	    /* Wait the reading of the Master for the fist half of the messages */
	    sem_reserve_1(master_sem_id, WAIT_END_ROUND);


	    

	    moves_to_master.mtype = player_type;
	    moves_to_master.points = total_used_moves;


	    if(msgsnd(master_msg_id, &moves_to_master, TO_PLAYER, 0) == -1){
			fprintf(stderr, "Failed Message Send PUNTEGGIO#%d: %s\n", errno, strerror(errno));
		}


		for(i = 0; i < parameters->SO_NUM_P; i++){
	    	free(pawns[i].target);
	    	free(pawns[i].temp_target);
	    	pawns[i].assigned = 0;
	    	pawns[i].temp_assigned = 0;
    	}
	}
}

int set_pawns(int letter, int parameters_id, int player_msg_id, int chessboard_mem_id, int chessboard_sem_id, int rows, int columns, struct pawn * pawns){
	static int num_pawn = 1;
	
	int positions_id;
	int * positions;
	int current_pos;
	int * chessboard;

	struct message message_to_pawn;
	int players;
	int pawns_one_player;
	struct param * parameters;
	int success;
	int pawns_number;
	int x;
	int y;
	int i;

	parameters = shmat(parameters_id,NULL,0);

	pawns_number = parameters->SO_NUM_G * parameters->SO_NUM_P;

	positions_id = shmget(POSITIONS_MEM_KEY,sizeof(int) * pawns_number, 0666 | IPC_CREAT);
    positions = shmat(positions_id,NULL,0);

    chessboard = shmat(chessboard_mem_id,NULL,0);

    

    players = parameters->SO_NUM_G;
    pawns_one_player = parameters->SO_NUM_P;
    srand(getpid());

    while(1){
    	i = rand() % (players * pawns_one_player);
    	success = sem_reserve_1_no_wait(chessboard_sem_id, positions[i]);
    	if(success != -1){
    		current_pos = positions[i];

    		pawns[num_pawn - 1].type = message_to_pawn.mtype = num_pawn;
    		
    		message_to_pawn.x = current_pos % columns;
    		message_to_pawn.y = (current_pos - message_to_pawn.x) / columns;
    		
    		chessboard[current_pos] = -letter;
    		break;
    	}
    }

    if((success = msgsnd(player_msg_id, &message_to_pawn, LEN_X_Y, 0)) == -1){
    	fprintf(stderr, "Failed Message Send#%d: %s\n", errno, strerror(errno));
    }
    
    num_pawn++;

    return message_to_pawn.y * columns + message_to_pawn.x;
}

int all_checked_flag(struct flag * flags, int flags_number){
	int i;
	for(i = 0; i < flags_number; i++){
		if(flags[i].checked == 0) return 0;
	}

	return 1;
}

void sigint_handler(int signal){
	int i;
	
    for(i = 0; i < pawns_number; i++){
        kill(pawns[i].pid, SIGINT);
    }

    
    while(wait(NULL) != -1);
    

    semctl(player_sem_id, 0, IPC_RMID);
    msgctl(player_msg_id, IPC_RMID, NULL);
    msgctl(player_msg_id_results, IPC_RMID, NULL);

    exit(EXIT_SUCCESS);
}

