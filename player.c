#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include "enviroment.h"
#include "semaphore.h"


int main(){

	
	int i;
	
	int * parameters;
	
	int sem_id;
	pid_t select;

	int master_sem_id;
	int a;
	
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	for (i = 0; i < 3; ++i){
		switch(fork()){
		case -1:
    		fprintf(stderr, "Failed to Fork Pawns PID#%d%s\n", getpid());
    		exit(EXIT_FAILURE);
    	case 0:
    		/*fprintf(stderr,"FIGLIO PID#%d\n", getppid());*/

    		exit(EXIT_SUCCESS);
    	default:
    		break;
		}
	}

	if((master_sem_id = semget(KEY_SEM_MASTER_WAIT_PLRS, 1, 0666)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Semaphore doesn't exist\n");
		exit(EXIT_FAILURE);
	}
	printf("Semaphore READY_ENTRY value: %d\n",semctl(master_sem_id, READY_ENTRY, GETVAL));

    sem_reserve_1(master_sem_id, 0);
    /*fprintf(stderr, "a: %d, %d%s\n", a, errno, strerror(errno));*/
        

    printf("Semaphore READY_ENTRY value DOPO: %d\n",semctl(master_sem_id, READY_ENTRY, GETVAL));

    while((select = wait(NULL)) != -1);

    /*printf("Risorsa liberata %d\n", getpid());*/
    
	return 0;
}