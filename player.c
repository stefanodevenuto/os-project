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

struct pawn{
    int type;
    int x;
    int y;
    int starting_x;
    int starting_y;
    int remaining_moves;
    int * target;
    int * temp_target;
    int assigned;
};

int set_pawns(int letter, int parameters_id, int player_msg_id, int chessboard_mem_id, int chessboard_sem_id, int rows, int columns, struct pawn * pawns);
struct position * calculate_position(int parameters_id);

int main(int argc, char *argv[]){

	int i;
	int k;
	
	int parameters_id;
	struct param * parameters;
	
	pid_t select;

	int master_sem_id;
	int player_sem_id;

	int player_msg_id;
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
	struct strategy strategy_pawn;

	char sprintf_parameters_id[200];
	char sprintf_letter[200];

	char * strat;

	int test;

	int turn_sem_id;

	int turn_sem_entry;
	int j;
	struct position * positions;
	int positions_id;

	struct pawn * pawns;

	int * flags;
	int * copy_flags;

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

	master_sem_id = semget(MAIN_SEM, 4, 0666);
    

	
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

	chessboard = shmat(chessboard_mem_id,NULL,0);

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


    pawns = malloc(sizeof(struct pawn) * parameters->SO_NUM_P);
    
    

    
    
    

   
    					/* Critical section players */
    
    /* --------------------------------------------------------------------- */
    for(i = 0; i < parameters->SO_NUM_P; i++){
    	pawns[i].type = i+1;
    	sem_reserve_1(turn_sem_id, (turn_sem_entry-1 + parameters->SO_NUM_G)% parameters->SO_NUM_G);
			position = set_pawns(player_type, parameters_id, player_msg_id, chessboard_mem_id, chessboard_sem_id, rows, columns, pawns);
	    sem_release(turn_sem_id, turn_sem_entry);

	    pawns[i].remaining_moves = parameters->SO_N_MOVES;
	    pawns[i].starting_x = pawns[i].x = position % columns;
	    pawns[i].starting_y = pawns[i].y = (position - pawns[i].x) / columns;
	    pawns[i].assigned = 0;
	    printf("(%d,%d)\n", pawns[i].x, pawns[i].y);
	    


	}
    /* --------------------------------------------------------------------- */

	

    sprintf (sprintf_letter, "%d", player_type);
	args[3] = sprintf_letter;


    

    
    
    						/* Fork the pawns*/
    /* -------------------------------------------------------------------------- */
	for (i = 0; i < parameters->SO_NUM_P; i++){
		switch(fork()){
		case -1:
    		fprintf(stderr, "Failed to Fork Pawns PID#%d%s\n", getpid());
    		exit(EXIT_FAILURE);
    		break;
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
    target_count = 0;
    
    /* Array of Flags (position => number of flag, value => position in the matrix)*/

    
    flags = malloc(sizeof(int));
    

    for(i = 0; i < rows; i++){
    	for(j = 0; j < columns; j++){
    		if(chessboard[i * columns + j] > 0){
    			flags[flags_number] = i * columns + j;
    			flags_number++;
    			copy_flags = realloc(flags, sizeof(int) * (i + 2));
    			flags = copy_flags;
    		}
    	}
    }

    for(i = 0; i < flags_number; i++ ){
    	printf("Flag: (%d,%d) => %d\n", flags[i] % columns, (flags[i] - flags[i] % columns) / columns), flags[i];
    }

    for(i = 0; i < parameters->SO_NUM_P; i++){
		pawns[i].target = (int *) malloc(sizeof(int) * flags_number);
		pawns[i].temp_target = (int *) malloc(sizeof(int) * flags_number);

    }




    /* Set of all target-entry to 0 */
    for(i = 0; i < parameters->SO_NUM_P; i++){
    	for(j = 0; j < flags_number; j++){
    		pawns[i].target[j] = 0;
    		pawns[i].temp_target[j] = 0;
    	}
    }



    /*----------------------------ALGORITMO--------------------------------------*/
    
	    for(i = 0; i < flags_number; i++){
	    	flag_column = flags[i] % columns;
	    	flag_row = (flags[i] - flag_column) / columns;
	    	min_distance = INT_MAX;
	    	target_count = 0;
	    	for(j = 0; j < parameters->SO_NUM_P; j++){
	    		pawn_column = pawns[j].x;
	    		pawn_row = pawns[j].y;

	    		distance = abs(pawn_column - flag_column) + abs(pawn_row - flag_row);

	    		if(pawns[j].remaining_moves >= distance && distance < min_distance){

	    			min_distance = distance;
	    			target_index = j;
	    			
	    		}

	    	}
	    	
	    	if(min_distance != INT_MAX){
	    		while(pawns[target_index].target[target_count] != 0){
	    			target_count++;
		    	}

		    	pawns[target_index].remaining_moves -= min_distance;

		    	pawns[target_index].target[target_count] = flags[i];

		    	pawns[target_index].assigned += 1;
		    	
		    	pawn_column = pawns[target_index].x;
		    	pawn_row = pawns[target_index].y;

		    	pawns[target_index].x = flag_column;
		    	pawns[target_index].y = flag_row;

		    	target_index = k;
	    	}
	    }

	
	
    /*-----------------------------------------------------------------------------------*/



    for(i = 0; i < parameters->SO_NUM_P; i++){
    	if(pawns[i].assigned > 0){
    		
    		for(j = 0; j < (pawns[i].assigned - 1); j++){
    		
    			flag_column = pawns[i].target[j] % columns;
    			flag_row = (pawns[i].target[j] - flag_column) / columns;

    			distance = abs(pawns[i].starting_x - flag_column) + abs(pawns[i].starting_y - flag_row);
    			
    			flag_column = pawns[i].target[j + 1] % columns;
    			flag_row = (pawns[i].target[j + 1] - flag_column) / columns;

    			distance_2 = abs(pawns[i].starting_x - flag_column) + abs(pawns[i].starting_y - flag_row);

    			printf("CAMBIO\n");

    			if(distance_2 < distance){
    				tmp = pawns[i].target[j + 1];
    				pawns[i].target[j + 1] = pawns[i].target[j];
    				pawns[i].target[j] = tmp;
    			}

    		
    		}
    	}
    }

    



    for(i = 0; i < parameters->SO_NUM_P; i++){
    	if(pawns[i].assigned > 0){
	    	printf("Player: %c-------------------------- PEDINA %d (%d,%d) ------------------------\n",player_type, i,pawns[i].x,pawns[i].y );	
	    	for(j = 0; j < flags_number; j++){
	    		if(pawns[i].target[j] != 0)
	    			printf("Player: %c %d: Target #%d => %d Moves: %d\n",player_type, i, j,pawns[i].target[j], pawns[i].remaining_moves);
	    		else
	    			break;
	    	}
    	}
    }
	
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
	sem_set_val(master_sem_id, START, parameters->SO_NUM_P * parameters->SO_NUM_G);
	/* -------------------------------------------------------------------------- */
    

    while((select = wait(NULL)) != -1);


    for(i = 0; i < parameters->SO_NUM_P; i++){
    	fprintf(stderr,"Rimozione Target #%d\n",i );
    	free(pawns[i].target);
    	free(pawns[i].temp_target);
    }

    fprintf(stderr,"Rimozione Flags\n");

    free(flags);
    fprintf(stderr,"Rimozione Pawns\n");
    free(pawns);
    

    semctl(player_sem_id, 0, IPC_RMID);
    msgctl(player_msg_id, IPC_RMID, NULL);
    
    
	exit(EXIT_SUCCESS);
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
    		message_to_pawn.mtype = num_pawn;
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
