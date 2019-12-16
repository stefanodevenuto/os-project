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
#include "semaphore.h"

int set_parameters();

int main(int argc, char const *argv[]){

    
    int chessboard_rows;
    int chessboard_cols;

    int processes_number;
    int index_child;
    /*char buffer[2];*/
    
    int num_bytes;
    int parameters_id;
    struct param * parameters;
    char sprintf_parameters_id[40];
    char sprintf_number_player[40];


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
    char sprintf_sem_entry[40];

    char * args[5]; 

    int flag_number;
    int total_score;
    int media;
    int num;
    int pos;
    int sum;

    int switch_color_pawn;
	
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
    



	/*Creation of the Players*/
    for (index_child = 0; index_child < parameters->SO_NUM_G; index_child++){
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
			default:
				break;
    	}
    }


                        /* Setting semaphores*/
    /* ----------------------------------------------------------------------------*/
    master_sem_id = semget(MAIN_SEM, 3, 0666 | IPC_CREAT);
    
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
        num = rand() % media + 1;
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
    sem_set_val(master_sem_id, A, parameters->SO_NUM_G);
    sem_set_val(master_sem_id, SYNCHRO, parameters->SO_NUM_G);
    
    sem_reserve_0(master_sem_id, A);
    /* -------------------------------------------------------------------- */


                    /* Unblock players and START GAME */
    /* -------------------------------------------------------------------- */
    sem_set_val(master_sem_id, MASTER, parameters->SO_NUM_G);    
    /* -------------------------------------------------------------------- */

    printf("Values:\n");
    for(i=0; i<rows; i++){
        
        for(j=0; j<columns; j++){
            switch_color_pawn = chessboard[i * parameters->SO_BASE + j];
            if( switch_color_pawn < 0){
                switch(switch_color_pawn){
                    case -65:
                        printf("\033[1;31m");
                        printf(" %c", -(chessboard[i * parameters->SO_BASE + j]));
                        printf("\033[0m");
                    break;
                    case -66:
                        printf("\033[1;34m");
                        printf(" %c", -(chessboard[i * parameters->SO_BASE + j]));
                        printf("\033[0m");
                    break;
                    case -67:
                        printf("\033[1;33m");
                        printf(" %c", -(chessboard[i * parameters->SO_BASE + j]));
                        printf("\033[0m");
                    break;
                    case -68:
                        printf("\033[1;32m");
                        printf(" %c", -(chessboard[i * parameters->SO_BASE + j]));
                        printf("\033[0m");
                    break;
                    default:
                        printf(" %c", -(chessboard[i * parameters->SO_BASE + j]));
                    break;
                }
                
            }else if(switch_color_pawn > 0){
                printf(" %d", chessboard[i * parameters->SO_BASE + j]);
            }else{
                printf(" _");
            }
        }
        printf("\n");
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
    
    /* Wait the dead children*/
    while((select = wait(NULL)) != -1);
		/*printf("Process %d\n", select);*/

    
	return 0;
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

    fscanf(fd, "SO_NUM_G = %d\nSO_NUM_P = %d\nSO_MAX_TIME = %d\nSO_BASE = %d\nSO_ALTEZZA = %d\nSO_FLAG_MIN = %d\nSO_FLAG_MAX = %d\nSO_ROUND_SCORE = %d\nSO_N_MOVES = %d\nSO_MIN_HOLD_NSEC = %d", \
        &parameters->SO_NUM_G, &parameters->SO_NUM_P,&parameters->SO_MAX_TIME,&parameters->SO_BASE,&parameters->SO_ALTEZZA,&parameters->SO_FLAG_MIN,&parameters->SO_FLAG_MAX,&parameters->SO_ROUND_SCORE,&parameters->SO_N_MOVES,&parameters->SO_MIN_HOLD_NSEC);


    

    

    /*printf("%d\n",parameters->SO_NUM_G);
    printf("%d\n",parameters->SO_NUM_P);
    printf("%d\n",parameters->SO_MAX_TIME);
    printf("%d\n",parameters->SO_BASE);
    printf("%d\n",parameters->SO_ALTEZZA);
    printf("%d\n",parameters->SO_FLAG_MIN);
    printf("%d\n",parameters->SO_FLAG_MAX);
    printf("%d\n",parameters->SO_ROUND_SCORE);
    printf("%d\n",parameters->SO_N_MOVES);
    printf("%d\n",parameters->SO_MIN_HOLD_NSEC);

    printf("Numero giocatori: %d\n",parameters->SO_NUM_G);*/
    

    return parameters_id; /* Returns the id of parameters */
}