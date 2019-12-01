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
	int chessboard_mem_id;
	int * chessboard;
	int chessboard_sem_id;

	int player_msg_id;
	struct message message_to_pawn;
	long type;
	int player_letter;
	int a;

	parameters_id = atol(argv[1]);
	type = atol(argv[2]);
	player_letter = -atol(argv[3]);

	
	parameters = shmat(parameters_id,NULL,0);

	player_sem_id = semget(getppid(), 1, 0666);

	player_msg_id = msgget(getppid(), 0666);

	a = msgrcv(player_msg_id, &message_to_pawn, LEN_X_Y, type, 0);
	if(a == -1){
		fprintf(stderr, "MSGRCV: ret: %d, errno: %d, %s\n", a, errno, strerror(errno));
	}

	

    
	if((chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(int) * parameters->SO_ALTEZZA * parameters->SO_BASE, 0666)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed access to memory for pawns\n");
		exit(EXIT_FAILURE);
	}

    chessboard = shmat(chessboard_mem_id,NULL,0);

    
   
    chessboard[message_to_pawn.y * parameters->SO_BASE + message_to_pawn.x] = player_letter;
    
   

    chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, parameters->SO_ALTEZZA * parameters->SO_BASE, 0666);
    
    semctl(chessboard_sem_id, message_to_pawn.y * parameters->SO_BASE + message_to_pawn.x, SETVAL, 0);

	printf("Pawn SBLOCCA\n");
	sem_reserve_1(player_sem_id, 0);
	exit(EXIT_SUCCESS);

}
