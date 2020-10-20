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

int round_number;
struct param * parameters;
int chessboard_mem_id;
int chessboard_sem_id;
int master_sem_id;
int turn_sem_id;
int parameters_id;
int positions_id;
int master_msg_id;
int * chessboard;
int play_time;

typedef struct{
    int pid;
    int points;
    int used_moves;
} PLAYER;

PLAYER * players;

int set_parameters();
void print_chessboard(int * chessboard, int chessboard_sem_id,int parameters_id, int rows, int columns);
int calculate_position(int parameters_id,int chessboard_mem_id,int chessboard_sem_id, int rows, int columns);
void sig_alarm_handler(int signal);
void sigint_handler (int signal);

int main(int argc, char const *argv[]){

    struct sigaction sa;
    struct sigaction sigint;
    int chessboard_rows;
    int chessboard_cols;
    int test;

    int processes_number;
    int index_child;
    /*char buffer[2];*/
    
    int num_bytes;
    
    
    char sprintf_parameters_id[200];
    char sprintf_number_player[200];


	pid_t select;
	int rows;
  	int columns;

	


	int i,j;

	char y;

    

    
    char sprintf_sem_entry[200];

    char * args[5]; 

    int flag_number;
    int total_score;
    int media;
    int num;
    int pos;
    int sum;

    int switch_color_pawn;
    

    
    
    struct message_to_master player_points;
    struct pawn_flag receive_points;

    bzero(&sa, sizeof(sa));

    sa.sa_handler = sig_alarm_handler;

    sigaction(SIGALRM, &sa, NULL);


    bzero(&sigint, sizeof(sigint));

    sigint.sa_handler = sigint_handler;

    sigaction(SIGINT, &sigint, NULL);

    round_number = 0;
	

    
    /* Unset the buffering of the streams stdout and stderr*/
    setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	
	/* Get and Read the Parameters from the config.txt file and allocate a Share Memory for them */
	set_parameters();
    

    sprintf (sprintf_parameters_id, "%d", parameters_id);

    

    args[0] = "./player";
    args[1] = sprintf_parameters_id;
    /* Setting in for for fork the args[2]*/
    /* Setting in for loop to know in whic entry of semaphore you have to wait*/ 
    args[4] = NULL;


   
    /* Initializing Play Time */
    play_time = 0;


                /* Setting mutual exclusion semaphore for turns */
    /* -------------------------------------------------------------------- */
    turn_sem_id = semget(MUTUAL_TURN, parameters->SO_NUM_G, 0666 | IPC_CREAT);
    /* Used to implement the mutual exclusion for the players positioning phase*/
    semctl(turn_sem_id, parameters->SO_NUM_G-1, SETVAL, 1);
    /* -------------------------------------------------------------------- */
    
                        /* Setting up chessboard */
    /* -------------------------------------------------------------------- */

    rows = parameters->SO_ALTEZZA;
    columns = parameters->SO_BASE;

    chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(int) * rows * columns, 0666 | IPC_CREAT);
    chessboard = shmat(chessboard_mem_id,NULL,0);

    chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, rows * columns, 0666 | IPC_CREAT);

    master_msg_id = msgget(getpid(), 0666 | IPC_CREAT);

    for(i=0; i<rows; i++){
        for(j=0; j<columns; j++){
            chessboard[i * parameters->SO_BASE + i] = 0;
            semctl(chessboard_sem_id, i * parameters->SO_BASE + j, SETVAL, 1);
        }
    }
    /* -------------------------------------------------------------------- */

    players = (PLAYER *)malloc(sizeof(PLAYER) * parameters->SO_NUM_G);

    for(i = 0; i < parameters->SO_NUM_G; i++){
        players[i].points = 0;
        players[i].used_moves = 0;
    }

    positions_id = calculate_position(parameters_id,chessboard_mem_id, chessboard_sem_id, rows, columns);

	                   /* Create Main Semaphore */
    /* ----------------------------------------------------------------------------*/
    master_sem_id = semget(MAIN_SEM, 5, 0666 | IPC_CREAT);
    /* ----------------------------------------------------------------------------*/

    /*Creation of the Players*/
    for (index_child = 0; index_child < parameters->SO_NUM_G; index_child++){
    	switch(players[index_child].pid = fork()){
    		case -1:
    			fprintf(stderr, "Failed to Fork Players\n");
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


                /* Used for the wait-for-zero for the master by players*/
    /* ----------------------------------------------------------------------------*/
    sem_set_val(master_sem_id, SYNCHRO, 0);
    sem_set_val(master_sem_id, MASTER, parameters->SO_NUM_G);
    /* ----------------------------------------------------------------------------*/


                        


                        /* Wait players */    
    /* ----------------------------------------------------------------------------*/
    
    sem_reserve_0(master_sem_id, MASTER);
    
    /* ----------------------------------------------------------------------------*/

    while(1){
                    /* Calculation of flags and their scores*/
        /* -------------------------------------------------------------------- */
        total_score = parameters->SO_ROUND_SCORE;

        srand(getpid());

        flag_number = rand() % (parameters->SO_FLAG_MAX - parameters->SO_FLAG_MIN + 1) + parameters->SO_FLAG_MIN;

        sum  = 0;
        for(i = flag_number; i > 1 ; i--){
            media = total_score/i;
            num = (rand() % media)  + 1;
            pos = rand() % (rows * columns);
            if(chessboard[pos] == 0){
                sum += num;
                chessboard[pos] = num;
                total_score -= num;
            }else{
                i++;
            }
        }
        while(1){
            pos = rand() % (rows * columns);
            if(chessboard[pos] == 0){
                sum += total_score;
                chessboard[pos] = total_score;
                break;
            }
        }
        /* -------------------------------------------------------------------- */

        print_chessboard(chessboard,chessboard_sem_id,parameters_id, rows, columns);

                        /* Set semaphores and wait */
        /* -------------------------------------------------------------------- */
        sem_set_val(master_sem_id, A, parameters->SO_NUM_G);

        sem_set_val(master_sem_id, SYNCHRO, parameters->SO_NUM_G);
        
        sem_reserve_0(master_sem_id, A);
        /* -------------------------------------------------------------------- */


                        /* Unblock players and START GAME */
        /* -------------------------------------------------------------------- */
        alarm(parameters->SO_MAX_TIME);
        sem_set_val(master_sem_id, MASTER, parameters->SO_NUM_G); 
        /* -------------------------------------------------------------------- */


                        /* Wait the players messages*/
        /* -------------------------------------------------------------------- */
        for(i = 0; i < flag_number; i++){
            if((test = msgrcv(master_msg_id, &receive_points,TO_PLAYER, 0, 0)) == -1){
                fprintf(stderr, "MSGRCV MASTER FAILED: ret: %d, errno: %d, %s\n", test, errno, strerror(errno));
            }else{
                
                players[receive_points.mtype-65].points += receive_points.points;
            }
        }
        
        play_time += alarm(0);
        round_number++;

        /* Notificate the Player that the Master read the first half of the messages */
        sem_set_val(master_sem_id, WAIT_END_ROUND, parameters->SO_NUM_G);


        for (i = 0; i < parameters->SO_NUM_G; i++){
            if((test = msgrcv(master_msg_id, &receive_points,TO_PLAYER, 0, 0)) == -1){
                fprintf(stderr, "MSGRCV MASTER FAILED: ret: %d, errno: %d, %s\n", test, errno, strerror(errno));
            }else{
                
                players[receive_points.mtype-65].used_moves = receive_points.points;
            }
        }
        /* -------------------------------------------------------------------- */

    }
    
}

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


    if(columns > rows){
        perfect_matrix_columns = ceil(sqrt(pawns_number));
        
        perfect_matrix_rows = ceil(pawns_number / perfect_matrix_columns);
    }else{
        perfect_matrix_rows = ceil(sqrt(pawns_number));

        perfect_matrix_columns = ceil(pawns_number / perfect_matrix_rows);
    }
    

    if(perfect_matrix_columns > columns || perfect_matrix_rows > rows){
       
        perfect_matrix_columns = columns;
        perfect_matrix_rows = rows;
    }
    
    x_step = ceil(columns / perfect_matrix_columns);

    y_step = ceil(rows / perfect_matrix_rows);

    


    pawn_index = 0;
    first = 0;
    first_row = 0;

    init_x = ceil(x_step / 2);
    init_y = ceil(y_step / 2);



    
                /* To avoid the creation of a new fake row between a step */
    /* -------------------------------------------------------------------------------- */

    while((perfect_matrix_rows - 1) * y_step + init_y >= rows && init_y > 0){
        
        init_y -= 1;    
    }
    while((perfect_matrix_rows - 1) * y_step + init_y >= rows && y_step > 0){
        
        y_step -= 1;    
    }
    while((perfect_matrix_columns - 1) * x_step + init_x >= columns && init_x > 0){
        
        init_x -= 1;
    }
    while((perfect_matrix_columns - 1) * x_step + init_x >= columns && x_step > 0){
    
        x_step -= 1;
    }
    /* -------------------------------------------------------------------------------- */

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
    FILE *fd;
    int n;
    
    parameters_id = shmget(PARAMETERS_MEM_KEY,sizeof(struct param), 0666 | IPC_CREAT);
    parameters = shmat(parameters_id,NULL,0);

    

    

    fd = fopen("config.txt", "r");
    if(!fd){
        fprintf(stderr, "Errore apertura file config.txt\n");
        exit(EXIT_FAILURE);
    }

    printf("Master : Reading Settings...\n");

    n = fscanf(fd, "%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d", \
        &parameters->SO_NUM_G, &parameters->SO_NUM_P,&parameters->SO_MAX_TIME,&parameters->SO_BASE,&parameters->SO_ALTEZZA,&parameters->SO_FLAG_MIN,&parameters->SO_FLAG_MAX,&parameters->SO_ROUND_SCORE,&parameters->SO_N_MOVES,&parameters->SO_MIN_HOLD_NSEC);
    
}

void print_chessboard(int * chessboard, int chessboard_sem_id,int parameters_id, int rows, int columns){
    int i;
    int j;
    int switch_color_pawn;
    struct param * parameters;
    parameters = shmat(parameters_id,NULL,0);

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
    /*printf("Semaphore values:\n");
    for(i=0; i<rows; i++){
        for(j=0; j<columns; j++){
            if(semctl(chessboard_sem_id, i * parameters->SO_BASE + j, GETVAL)){
                printf("%d ", semctl(chessboard_sem_id, i * parameters->SO_BASE + j, GETVAL));
            }else{
                printf("\033[0;31m"); 
                printf("%d ", semctl(chessboard_sem_id, i * parameters->SO_BASE + j, GETVAL));
                printf("\033[0m");
            }
            

        }
        printf("\n");
    }*/
}

void sig_alarm_handler (int signal){
    int i;
    int j;
    int total_points;
    total_points = 0;
    printf("\n\n\n\t\t\t\t\t\tChessboard Status\n");
    print_chessboard(chessboard,chessboard_sem_id,parameters_id, parameters->SO_ALTEZZA, parameters->SO_BASE);
    printf("\n\n-------------------------- Statistics --------------------------\n\n");
    printf("\tRound Played:\t%d\n\n", round_number);
    printf("\tPlay Time:\t%f\n\n", (float)play_time);
    for(i = 0; i < parameters->SO_NUM_G; i++){
        printf("\t------------------- Player %c --------------------\n\n", i+65);
        printf("\tPoints:\t%d\n", players[i].points);
        printf("\tUsed Moves:\t%d\n", players[i].used_moves);
        printf("\tPercentage of Used Moves:\t%f\n", (float)players[i].used_moves / ((float)parameters->SO_N_MOVES * (float)parameters->SO_NUM_P));
        printf("\tPoints by Used Moves:\t%f\n", (float)players[i].points / (float)players[i].used_moves);
        total_points += players[i].points;
        printf("\t--------------------------------------------------\n\n");
    }
    printf("\tTotal points by Game Time:\t%f\n", (float)total_points / (float)play_time);
    printf("\n\n----------------------------------------------------------------\n\n");


    for(i = 0; i < parameters->SO_NUM_G; i++){
        kill(players[i].pid, SIGINT);
    }

    
    while(wait(NULL) != -1);
    
    semctl(chessboard_sem_id, 0, IPC_RMID);
    semctl(master_sem_id, 0, IPC_RMID);
    semctl(turn_sem_id, 0, IPC_RMID);
    shmctl(parameters_id, IPC_RMID, NULL);
    shmctl(chessboard_mem_id, IPC_RMID, NULL);
    shmctl(positions_id, IPC_RMID, NULL);
    msgctl(master_msg_id, IPC_RMID, NULL);

    exit(EXIT_SUCCESS);
}

void sigint_handler (int signal){
    printf("\n\nRound Played Before Stopping:\t%d\n\n", round_number);

    while(wait(NULL) != -1);
    
    semctl(chessboard_sem_id, 0, IPC_RMID);
    semctl(master_sem_id, 0, IPC_RMID);
    semctl(turn_sem_id, 0, IPC_RMID);
    shmctl(parameters_id, IPC_RMID, NULL);
    shmctl(chessboard_mem_id, IPC_RMID, NULL);
    shmctl(positions_id, IPC_RMID, NULL);
    msgctl(master_msg_id, IPC_RMID, NULL);

    exit(EXIT_SUCCESS);
}