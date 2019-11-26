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
	
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	set_env();
    parameters = get_env();
	
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


	if(sem_id = semget(KEY, 1, 0666) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Semaphore doesn't exist\n");
		exit(EXIT_FAILURE);
	}


    sem_reserve_1(sem_id, READY);
    printf("Semaphore READY value: %d\n",semctl(sem_id, READY, GETVAL));

    while((select = wait(NULL)) != -1);

    /*printf("Risorsa liberata %d\n", getpid());*/
    free(parameters);
    
	return 0;
}