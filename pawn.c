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
	int parameters_id;
	struct param * parameters;
	int player_sem_id;

	parameters_id = atol(argv[1]);
	parameters = shmat(parameters_id,NULL,0);

	player_sem_id = semget(getppid(), 1, 0666 | IPC_CREAT);

	sleep(3);

	printf("Pawn SBLOCCA\n");
	sem_reserve_1(player_sem_id, 0);
	exit(EXIT_SUCCESS);

}
