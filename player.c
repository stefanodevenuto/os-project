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


int main(int argc, char *argv[]){
	

	if(argc < 2){
		fprintf(stderr, "Failed to pass parameter to players%s\n");
		exit(EXIT_FAILURE);
	}
	
	int i;
	
	int parameters_id;
	struct param * parameters;
	
	pid_t select;

	int master_sem_id;
	int player_sem_id;

	int a;

	parameters_id = atoi(argv[1]);
	
	
	parameters = shmat(parameters_id,NULL,0);
	
	
	
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	/*printf("KEY_SEM_PLRS_WAIT_PWNS: %d, \n", KEY_SEM_PLRS_WAIT_PWNS);*/
	
	if((player_sem_id = semget(getpid(), 1, 0666 | IPC_CREAT)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Semaphore doesn't exist\n");
		exit(EXIT_FAILURE);
	}
	
    
    sem_set_val(player_sem_id, READY_ENTRY, parameters->SO_NUM_P);
    

	for (i = 0; i < parameters->SO_NUM_P; i++){
		switch(fork()){
		case -1:
    		fprintf(stderr, "Failed to Fork Pawns PID#%d%s\n", getpid());
    		exit(EXIT_FAILURE);
    	case 0:
    		if(execve("./pawn", argv, NULL)){
    			fprintf(stderr, "Execve() failed #%d : %s\n", errno, strerror(errno));
    			exit(EXIT_FAILURE);
    		}
    	default:
    		break;
		}
	}

	if((master_sem_id = semget(KEY_SEM_MASTER_WAIT_PLRS, 1, 0666)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Semaphore doesn't exist\n");
		exit(EXIT_FAILURE);
	}

    printf("Player aspetta %d\n", getpid());
    sem_reserve_0(player_sem_id, 0);
    printf("Player SINCRONIZZATO\n");

    sleep(3);
    sem_reserve_1(master_sem_id, 0);
    /*fprintf(stderr, "a: %d, %d%s\n", a, errno, strerror(errno));*/
        
    while((select = wait(NULL)) != -1);

    /*printf("Risorsa liberata %d\n", getpid());*/
    
	exit(EXIT_SUCCESS);
}