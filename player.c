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

	int player_msg_id;
	char sprintf_type[40];

	int a;

	char * args[4]; 

	struct message message_to_pawn;

	parameters_id = atoi(argv[1]);
	
	
	parameters = shmat(parameters_id,NULL,0);

	char sprintf_parameters_id[40];
    sprintf (sprintf_parameters_id, "%d", parameters_id);

    

    args[0] = "./player";
    args[1] = sprintf_parameters_id;
    /* wait type on args[2]*/
    args[3] = NULL;
	
	
	
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	/*SETTING OF THE WAIT FOR 0 SEMAPHORE FOR EVERY PAWN FOR EVERY PLAYER*/
	
	if((player_sem_id = semget(getpid(), 1, 0666 | IPC_CREAT)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed semaphore for pawns\n");
		exit(EXIT_FAILURE);
	}
	
    sem_set_val(player_sem_id, READY_ENTRY, parameters->SO_NUM_P);

    /*SETTING THE MESSAGE QUEUE FOR EVERY PLAYER*/
    player_msg_id = msgget(getpid(), 0666 | IPC_CREAT);

    for(i = 1; i <= parameters->SO_NUM_P; i++){
    	message_to_pawn.mtype = i;
	    message_to_pawn.x = i;
	    message_to_pawn.y = i;
	    /*message_to_pawn.strategy = "NSWEEWSN"*/

	    a = msgsnd(player_msg_id, &message_to_pawn, sizeof(int) * 2, 0);
	    if(a == -1){
	    	fprintf(stderr, "MSGSEND: ret: %d, errno: %d, %s\n", a, errno, strerror(errno));
	    }
    }

    
    /*-------------------------------------------*/

    

	for (i = 0; i < parameters->SO_NUM_P; i++){
		switch(fork()){
		case -1:
    		fprintf(stderr, "Failed to Fork Pawns PID#%d%s\n", getpid());
    		exit(EXIT_FAILURE);
    	case 0:
    		
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