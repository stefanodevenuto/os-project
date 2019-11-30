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
	int parameters_id;
	struct param * parameters;
	int player_sem_id;

	int player_msg_id;
	struct message message_to_pawn;
	long type;
	int a;

	parameters_id = atol(argv[1]);
	type = atol(argv[2]);
	
	parameters = shmat(parameters_id,NULL,0);

	player_sem_id = semget(getppid(), 1, 0666);

	player_msg_id = msgget(getppid(), 0666);

	a = msgrcv(player_msg_id, &message_to_pawn, LEN_X_Y, type, 0);
	if(a == -1){
		fprintf(stderr, "MSGRCV: ret: %d, errno: %d, %s\n", a, errno, strerror(errno));
	}

	printf("X: %d, Y: %d\n",message_to_pawn.x, message_to_pawn.y );

	printf("Pawn SBLOCCA\n");
	sem_reserve_1(player_sem_id, 0);
	exit(EXIT_SUCCESS);

}
