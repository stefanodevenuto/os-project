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
#include <sys/types.h>
#include <signal.h>
#include <math.h>
#include "semaphore.h"

int set_parameters();
void print_chessboard(int * chessboard, int chessboard_sem_id,int parameters_id, int rows, int columns);
int calculate_position(int parameters_id,int chessboard_mem_id,int chessboard_sem_id, int rows, int columns);
void handle_signal(int signal);

int main(int argc, char const *argv[]){

    struct sigaction sa;
    int chessboard_rows;
    int chessboard_cols;
    int test;

    int processes_number;
    int index_child;
    /*char buffer[2];*/
    
    int num_bytes;
    int parameters_id;
    struct param * parameters;
    char sprintf_parameters_id[200];
    char sprintf_number_player[200];


	pid_t select;
	int rows;
  	int columns;

	int * chessboard;


	int i,j;

	char y;

    int chessboard_mem_id;
    int chessboard_sem_id;

    int master_sem_id;
    int turn_sem_id;
    char sprintf_sem_entry[200];

    char * args[5]; 

    int flag_number;
    int total_score;
    int media;
    int num;
    int pos;
    int sum;

    int switch_color_pawn;
    int positions_id;

    bzero(&sa, sizeof(sa));

    sa.sa_handler = handle_signal;

    sigaction(SIGINT, &sa, NULL);
	
    /*int semid;
    char my_string[100];*/

    /* Setting the enviroment variable 
    set_env();*/
printf("PID MAster: %d\n", getpid());
    /* Unset the buffering of the streams stdout and stderr*/
    setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	
	/* Get and parse of the set enviroment */
	
    parameters_id = set_parameters();
    parameters = shmat(parameters_id,NULL,0);

    sprintf (sprintf_parameters_id, "%d", parameters_id);

    

    args[0] = "./player";
    args[1] = sprintf_parameters_id;
    /* Setting in for for fork the args[2]*/
    /* Setting in for loop to know in whic entry of semaphore you have to wait*/ 
    args[4] = NULL;


    /*print_chessboard();


                /* Setting mutual exclusion semaphore for turns */
    /* -------------------------------------------------------------------- */
    turn_sem_id = semget(MUTUAL_TURN, parameters->SO_NUM_G, 0666 | IPC_CREAT);
    semctl(turn_sem_id, parameters->SO_NUM_G-1, SETVAL, 1);
    /* -------------------------------------------------------------------- */
    
                        /* Setting up chessboard */
    /* -------------------------------------------------------------------- */

    rows = parameters->SO_ALTEZZA;
    columns = parameters->SO_BASE;

    chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(int) * rows * columns, 0666 | IPC_CREAT);
    chessboard = shmat(chessboard_mem_id,NULL,0);

    chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, rows * columns, 0666 | IPC_CREAT);

    for(i=0; i<rows; i++){
        for(j=0; j<columns; j++){
            chessboard[i * parameters->SO_BASE + i] = 0;
            semctl(chessboard_sem_id, i * parameters->SO_BASE + j, SETVAL, 1);
        }
    }
    /* -------------------------------------------------------------------- */

    positions_id = calculate_position(parameters_id,chessboard_mem_id, chessboard_sem_id, rows, columns);

	/*Creation of the Players*/
    for (index_child = 0; index_child < parameters->SO_NUM_G; index_child++){
        printf("%d\n", parameters->SO_NUM_G);
    	switch(fork()){
    		case -1:
    			fprintf(stderr, "Failed to Fork Players%s\n");
    			exit(EXIT_FAILURE);
    		case 0:
                sprintf (sprintf_number_player, "%d", 64 + index_child+1);
                args[2] = sprintf_number_player;
                sprintf (sprintf_sem_entry, "%d", index_child);
                args[3] = sprintf_sem_entry;
    			if(execve("./player", args, NULL)){
    				fprintf(stderr, "Execve() failed #%d : %s\n", errno, strerror(errno));
    				exit(EXIT_FAILURE);
    			}
                printf("HO FALLITO A SPAWNARE GIOCATORE %d\n", getpid());
                exit(EXIT_FAILURE);
			default:
				break;
    	}
    }


                        /* Setting semaphores*/
    /* ----------------------------------------------------------------------------*/
    master_sem_id = semget(MAIN_SEM, 4, 0666 | IPC_CREAT);
    
    /* Used for the wait-for-zero for the master by players*/
    sem_set_val(master_sem_id, SYNCHRO, 0);
    sem_set_val(master_sem_id, MASTER, parameters->SO_NUM_G);

    /* Used to implement the mutual exclusion for the players positioning phase*/
    
    /* ----------------------------------------------------------------------------*/


                        


                        /* Wait players */    
    /* ----------------------------------------------------------------------------*/
    printf("Master aspetta %d\n", MAIN_SEM);

    sem_reserve_0(master_sem_id, MASTER);
    
    fprintf(stdout,"Master SINCRONIZZATO\n");
    /* ----------------------------------------------------------------------------*/

                /* Calculation of flags and their scores*/
    /* -------------------------------------------------------------------- */
    total_score = parameters->SO_ROUND_SCORE;

    srand(getpid());

    flag_number = rand() % (parameters->SO_FLAG_MAX - parameters->SO_FLAG_MIN + 1) + parameters->SO_FLAG_MIN;

    sum  = 0;
    for(i = flag_number - 1; i != 0 ; i--){
        media = total_score/i;
        num = rand() % media  + 1;
        pos = rand() % (rows * columns);
        if(chessboard[pos] == 0){
            sum += num;
            chessboard[pos] = num;
            total_score -= num;
            printf("%d \n", num);
        }else{
            i++;
        }
    }
    while(1){
        pos = rand() % (rows * columns);
        if(chessboard[pos] == 0){
            sum += total_score;
            chessboard[pos] = total_score;
            printf("%d \n", total_score);
            break;
        }
    }
    printf("Somma: %d\n", sum);
    /* -------------------------------------------------------------------- */


                    /* Set semaphores and wait */
    /* -------------------------------------------------------------------- */
    printf("Setto A a %d\n", parameters->SO_NUM_G);
    sem_set_val(master_sem_id, A, parameters->SO_NUM_G);
    printf("--------------------------Setto SYNCHRO a %d\n", parameters->SO_NUM_G);
    sem_set_val(master_sem_id, SYNCHRO, parameters->SO_NUM_G);
    printf("SYNCHRO e' settato a:  %d\n", semctl(master_sem_id, SYNCHRO, GETVAL));
    
    sem_reserve_0(master_sem_id, A);
    /* -------------------------------------------------------------------- */


                    /* Unblock players and START GAME */
    /* -------------------------------------------------------------------- */
    sem_set_val(master_sem_id, MASTER, parameters->SO_NUM_G);    
    /* -------------------------------------------------------------------- */

    print_chessboard(chessboard,chessboard_sem_id,parameters_id, rows, columns);
    
    /* Wait the dead children*/
    while((select = wait(NULL)) != -1);
		/*printf("Process %d\n", select);*/
    printf("STO PER CANCELLARE TUTTOOOOOOOOOOO\n");
    test = semctl(chessboard_sem_id, 0, IPC_RMID);
    fprintf(stderr,"test: %d, Errno: %d: %s\n", test, errno, strerror(errno));
    semctl(master_sem_id, 0, IPC_RMID);
    semctl(turn_sem_id, 0, IPC_RMID);
    shmctl(parameters_id, IPC_RMID, NULL);
    shmctl(chessboard_mem_id, IPC_RMID, NULL);
    shmctl(positions_id, IPC_RMID, NULL);

	return 0;
}

/*int calculate_position(int parameters_id,int chessboard_mem_id,int chessboard_sem_id, int rows, int columns){
    int i;
    int j;
    int pawns_number;
    int step;
    int * chessboard;
    struct param * parameters;
    int count;
    int pawn_counter;
    int num_pawn;
    int positions_id;
    int * positions;
    int index_pos;

    
    parameters = shmat(parameters_id,NULL,0);
    chessboard = shmat(chessboard_mem_id,NULL,0);

    positions_id = shmget(POSITIONS_MEM_KEY,sizeof(int) * rows * columns, 0666 | IPC_CREAT);
    positions = shmat(positions_id,NULL,0);


    

    j = 0;
    pawns_number = parameters->SO_NUM_P * parameters->SO_NUM_G;
    

    step = (rows * columns) / pawns_number;
    

    num_pawn = 0;
    count = 1;
    index_pos = 0;
    for(j = 0; j < columns; j++){
        if(num_pawn == pawns_number) break;
        for(i = 0; i< rows; i++){
            if(count == step){
                if(num_pawn == pawns_number) break;
                else{
                    positions[num_pawn] = i * columns + j;
                    count = 1;
                    num_pawn++;
                }
                
            }else count++;
        }
    }

    return positions_id;
}*/

int calculate_position(int parameters_id,int chessboard_mem_id,int chessboard_sem_id, int rows, int columns){
    struct param * parameters;
    float perfect_matrix_rows;
    float perfect_matrix_columns;
    int positions_id;
    int * positions;
    int pawns_number;
    int * chessboard;
    int pawn_index;
    int x_step;
    int y_step;
    int i;
    int j;
    int first;
    int first_row;

    int t_x;
    int t_y;
    int pawns_per_row;
    int pawns_per_column;
    int init_x;
    int init_y;
    

    parameters = shmat(parameters_id,NULL,0);
    chessboard = shmat(chessboard_mem_id,NULL,0);

    pawns_number = parameters->SO_NUM_P * parameters->SO_NUM_G;

    if(pawns_number > rows * columns){
        fprintf(stderr, "*** Error: Number of Pawns greater of the Chessboard Size\n");
        exit(EXIT_FAILURE);
    }

    positions_id = shmget(POSITIONS_MEM_KEY,sizeof(int) * pawns_number, 0666 | IPC_CREAT);
    positions = shmat(positions_id,NULL,0);


    /* 2 Player x 7 pawns */

    printf("pawns_number: %d\n", pawns_number);
    printf("rad of pawns_number: %f, ceil: %f\n", sqrt(pawns_number), ceil(sqrt(pawns_number)));

    if(columns > rows){
        perfect_matrix_columns = ceil(sqrt(pawns_number));
        
        perfect_matrix_rows = ceil(pawns_number / perfect_matrix_columns);
    }else{
        perfect_matrix_rows = ceil(sqrt(pawns_number));

        perfect_matrix_columns = ceil(pawns_number / perfect_matrix_rows);
    }
    

    if(perfect_matrix_columns > columns || perfect_matrix_rows > rows){
        printf("TURBANCE\n");
        perfect_matrix_columns = columns;
        perfect_matrix_rows = rows;
    }
    
    x_step = ceil(columns / perfect_matrix_columns);

    y_step = ceil(rows / perfect_matrix_rows);

    printf("perfect_matrix_columns: %f, perfect_matrix_rows: %f\n", perfect_matrix_columns, perfect_matrix_rows);

    printf("x_step: %d, y_step: %d\n", x_step, y_step);

    


    pawn_index = 0;
    first = 0;
    first_row = 0;

    init_x = ceil(x_step / 2);
    init_y = ceil(y_step / 2);

    printf("PRIMA: init_x: %d, init_y: %d\n",init_x, init_y );
    printf("PRIMA: x_step: %d, y_step: %d\n",x_step, y_step );



    /* DA CAPIRE QUANDO PREFERIRE DECREMENTARE PRIMA init_y E DOPO y_step O VICEVERSA */
    /* To avoid the creation of a new fake row between a step */
    /* -------------------------------------------------------------------------------- */
    printf("columns: %d\n", columns);

    while((perfect_matrix_rows - 1) * y_step + init_y >= rows && init_y > 0){
        printf("INIT_Y\n");
        init_y -= 1;    
    }
    while((perfect_matrix_rows - 1) * y_step + init_y >= rows && y_step > 0){
        printf("Y_STEP\n");
        y_step -= 1;    
    }
    while((perfect_matrix_columns - 1) * x_step + init_x >= columns && init_x > 0){
        printf("INIT_X\n");
        init_x -= 1;
    }
    while((perfect_matrix_columns - 1) * x_step + init_x >= columns && x_step > 0){
        printf("X_STEP\n");
        printf("columns: %d\n", columns);
        x_step -= 1;
    }
    /* -------------------------------------------------------------------------------- */

    printf("DOPO: init_x: %d, init_y: %d\n",init_x, init_y );
    printf("DOPO: x_step: %d, y_step: %d\n",x_step, y_step );


    for(i=0; i<perfect_matrix_rows; i++){
        for(j=0; j<perfect_matrix_columns; j++){
            if(pawn_index < pawns_number){
                if(j == 0){
                    t_x = init_x;
                }else{
                    t_x = (positions[pawn_index - 1] % columns) + x_step;
                }
                t_y = init_y;
                positions[pawn_index] = t_y * columns + t_x;
                pawn_index++;
            }else break;
            
        }
        if(pawn_index >= pawns_number) break;

        if(y_step == 0)
            init_y += 1;
        else
            init_y += y_step;
    }

    return positions_id;
}

int set_parameters(){
    /*
    .
    .
    .
    . Read from file
    .
    .
    .
    */
    int parameters_id;
    struct param * parameters;
    FILE *fd;
    int a;
    int b;
    
    parameters_id = shmget(PARAMETERS_MEM_KEY,sizeof(struct param), 0666 | IPC_CREAT);
    parameters = shmat(parameters_id,NULL,0);

    

    

    fd = fopen("config.txt", "r");
    if(!fd){
        fprintf(stderr, "Errore apertura file config.txt\n");
        exit(EXIT_FAILURE);
    }

    printf("Sto leggendo...\n");

    fscanf(fd, "%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d", \
        &parameters->SO_NUM_G, &parameters->SO_NUM_P,&parameters->SO_MAX_TIME,&parameters->SO_BASE,&parameters->SO_ALTEZZA,&parameters->SO_FLAG_MIN,&parameters->SO_FLAG_MAX,&parameters->SO_ROUND_SCORE,&parameters->SO_N_MOVES,&parameters->SO_MIN_HOLD_NSEC);


    

    

    printf("%d\n",parameters->SO_NUM_G);
    printf("%d\n",parameters->SO_NUM_P);
    printf("%d\n",parameters->SO_MAX_TIME);
    printf("%d\n",parameters->SO_BASE);
    printf("%d\n",parameters->SO_ALTEZZA);
    printf("%d\n",parameters->SO_FLAG_MIN);
    printf("%d\n",parameters->SO_FLAG_MAX);
    printf("%d\n",parameters->SO_ROUND_SCORE);
    printf("%d\n",parameters->SO_N_MOVES);
    printf("%d\n",parameters->SO_MIN_HOLD_NSEC);

    printf("Numero giocatori: %d\n",parameters->SO_NUM_G);
    

    return parameters_id; /* Returns the id of parameters */
}

void print_chessboard(int * chessboard, int chessboard_sem_id,int parameters_id, int rows, int columns){
    int i;
    int j;
    int switch_color_pawn;
    struct param * parameters;
    parameters = shmat(parameters_id,NULL,0);

    printf("Values:\n");
    for(i=0; i<rows; i++){
        
        for(j=0; j<columns; j++){
            switch_color_pawn = chessboard[i * parameters->SO_BASE + j];
            if( switch_color_pawn < 0){
                switch(switch_color_pawn){
                    case -65:
                        fprintf(stderr, "\033[1;31m");
                        fprintf(stderr, " %c", -(chessboard[i * parameters->SO_BASE + j]));
                        fprintf(stderr, "\033[0m");
                    break;
                    case -66:
                        fprintf(stderr, "\033[1;34m");
                        fprintf(stderr, " %c", -(chessboard[i * parameters->SO_BASE + j]));
                        fprintf(stderr, "\033[0m");
                    break;
                    case -67:
                        fprintf(stderr, "\033[1;36m");
                        fprintf(stderr, " %c", -(chessboard[i * parameters->SO_BASE + j]));
                        fprintf(stderr, "\033[0m");
                    break;
                    case -68:
                        fprintf(stderr, "\033[1;32m");
                        fprintf(stderr, " %c", -(chessboard[i * parameters->SO_BASE + j]));
                        fprintf(stderr, "\033[0m");
                    break;
                    default:
                        fprintf(stderr, "  %c", -(chessboard[i * parameters->SO_BASE + j]));
                    break;
                }
                
            }else if(switch_color_pawn > 0){
                /*fprintf(stderr, " ⭐");*/
                fprintf(stderr, " %d", chessboard[i * parameters->SO_BASE + j]);
            }else{
                fprintf(stderr, " _");
            }
        }
        fprintf(stderr, "\n");
    }
    printf("Semaphore values:\n");
    for(i=0; i<rows; i++){
        for(j=0; j<columns; j++){
            if(semctl(chessboard_sem_id, i * parameters->SO_BASE + j, GETVAL)){
                printf("%d ", semctl(chessboard_sem_id, i * parameters->SO_BASE + j, GETVAL));
            }else{
                printf("\033[0;31m"); /* Change color to RED*/
                printf("%d ", semctl(chessboard_sem_id, i * parameters->SO_BASE + j, GETVAL));
                printf("\033[0m");
            }
            

        }
        printf("\n");
    }
}

void handle_signal(int signal){
    printf("HANDLER SIGINT\n");
}