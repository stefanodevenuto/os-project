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
    


	pid_t select;
	int rows;
  	int columns;

	char * chessboard;


	int i,j;

	char y;

    int chessboard_mem_id;
    int chessboard_sem_id;

    int master_sem_id;

    char * args[3]; 
	
    /*int semid;
    char my_string[100];*/

    /* Setting the enviroment variable 
    set_env();

    /* Unset the buffering of the streams stdout and stderr*/
    setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	
	/* Get and parse of the set enviroment */
	
    parameters_id = set_parameters();
    parameters = shmat(parameters_id,NULL,0);

    char sprintf_parameters_id[40];
    sprintf (sprintf_parameters_id, "%d", parameters_id);

    

    args[0] = "./player";
    args[1] = sprintf_parameters_id;
    args[2] = NULL;

    
    


    /*print_chessboard();

    
    /* Setting up chessboard and print*/
    /*--------------------------------------------------------------------*/

    rows = parameters->SO_ALTEZZA;
    columns = parameters->SO_BASE;

    chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(char) * rows * columns, 0666 | IPC_CREAT);
    chessboard = shmat(chessboard_mem_id,NULL,0);

    chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, rows * columns, 0666 | IPC_CREAT);

    for(i=0; i<rows; i++){
        for(j=0; j<columns; j++){
            chessboard[i * parameters->SO_BASE + i] = 'A';
            semctl(chessboard_sem_id, i * parameters->SO_BASE + i, SETVAL, 1);
        }
    }

    /*printf("Values:\n");
    for(i=0; i<rows; i++){
        for(j=0; j<columns; j++){
            printf("%c ", chessboard[i * parameters->SO_BASE + i]);
        }
        printf("\n");
    }
    printf("Semaphore values:\n");
    for(i=0; i<rows; i++){
        for(j=0; j<columns; j++){
            printf("%d ", semctl(chessboard_sem_id, i * parameters->SO_BASE + i, GETVAL));

        }
        printf("\n");
    }
    /*--------------------------------------------------------------------*/
    


    /*printf("player number: %d\n", parameters[SO_NUM_G]);
	/*Creation of the Players*/
    for (index_child = 0; index_child < parameters->SO_NUM_G; index_child++){
    	switch(fork()){
    		case -1:
    			fprintf(stderr, "Failed to Fork Players%s\n");
    			exit(EXIT_FAILURE);
    		case 0:
    			if(execve("./player", args, NULL)){
    				fprintf(stderr, "Execve() failed #%d : %s\n", errno, strerror(errno));
    				exit(EXIT_FAILURE);
    			}
			default:
				break;
    	}
    }

    /*printf("parameters[SO_NUM_G]: %d\n", parameters[SO_NUM_G]);*/
    
    master_sem_id = semget(KEY_SEM_MASTER_WAIT_PLRS, 1, 0666 | IPC_CREAT);
    
    sem_set_val(master_sem_id, READY_ENTRY, parameters->SO_NUM_G);
    

    /*printf("Ready PADRE: %d\n",semctl(sem_id, READY_ENTRY, GETVAL));*/
    printf("Master aspetta %d\n", KEY_SEM_MASTER_WAIT_PLRS);

    sem_reserve_0(master_sem_id, 0);
    /*printf("Ready PADRE: %d\n",semctl(sem_id, READY_ENTRY, GETVAL));*/
    fprintf(stdout,"Master SINCRONIZZATo\n");
    
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

    int a;
    int b;
    
    parameters_id = shmget(PARAMETERS_MEM_KEY,sizeof(struct param), 0666 | IPC_CREAT);
    parameters = shmat(parameters_id,NULL,0);

    

    

    FILE *fd = fopen("config.txt", "r");
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