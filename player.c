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

void set_pawns(int letter, int parameters_id, int player_msg_id, int chessboard_mem_id, int chessboard_sem_id);

int main(int argc, char *argv[]){

	printf("PID FIGLIO: %d\n", getpid());
	

	if(argc < 2){
		fprintf(stderr, "Failed to pass parameter to players%s\n");
		exit(EXIT_FAILURE);
	}
	
	int i;
	int k;
	
	int parameters_id;
	struct param * parameters;
	
	pid_t select;

	int master_sem_id;
	int player_sem_id;

	int player_msg_id;
	char sprintf_type[40];
	int player_type;

	int turn_semaphore;

	int chessboard_sem_id;
	int chessboard_mem_id;
	int rows;
	int columns;


	int a;

	char * args[5]; 

	struct message message_to_pawn;

	char sprintf_parameters_id[40];
	char sprintf_letter[40];

	parameters_id = atoi(argv[1]);
	player_type = atoi(argv[2]);

	/*printf("Player Tipe: %d, in char: %c\n", player_type,player_type);*/
	
	
	parameters = shmat(parameters_id,NULL,0);

	turn_semaphore = semget(KEY_SEM_MASTER_WAIT_PLRS, 2, 0666 | IPC_CREAT);
    

	
    sprintf (sprintf_parameters_id, "%d", parameters_id);

    

    args[0] = "./player";
    args[1] = sprintf_parameters_id;
    /* wait type on args[2]*/
    /* wait letter to pawn on args[3]*/
    args[4] = NULL;
	
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	rows = parameters->SO_ALTEZZA;
	columns = parameters->SO_BASE;

	chessboard_mem_id = shmget(CHESSBOARD_MEM_KEY,sizeof(int) * rows * columns, 0666);
	chessboard_sem_id = semget(CHESSBOARD_SEM_KEY, rows * columns, 0666);


	/*SETTING OF THE WAIT FOR 0 SEMAPHORE FOR EVERY PAWN FOR EVERY PLAYER*/

	/* sem_reserve_1(master_sem_id, 0); */
	
	if((player_sem_id = semget(getpid(), 1, 0666 | IPC_CREAT)) == -1){
		if(errno == ENOENT)
			fprintf(stderr, "Failed semaphore for pawns\n");
		exit(EXIT_FAILURE);
	}
	
    sem_set_val(player_sem_id, READY_ENTRY, parameters->SO_NUM_P);

    /*SETTING THE MESSAGE QUEUE FOR EVERY PLAYER*/
    player_msg_id = msgget(getpid(), 0666 | IPC_CREAT);

    i=0;
    int l = 0;

    						/* SEZIONE CRITICA GIOCATORI */
    /*-----------------------------------------------------------------------------------------*/
    sem_reserve_1(turn_semaphore, TURN_ENTRY);
	    set_pawns(player_type, parameters_id, player_msg_id, chessboard_mem_id, chessboard_sem_id);
	    sprintf (sprintf_letter, "%d", player_type);
		args[3] = sprintf_letter;
    	printf("FINITO PLAYER %c\n", player_type);
    	sleep(3);
    sem_release(turn_semaphore, TURN_ENTRY);
    /*-----------------------------------------------------------------------------------------*/


    

    
    /*-------------------------------------------*/

    

	for (i = 0; i < parameters->SO_NUM_P; i++){
		switch(fork()){
		case -1:
    		fprintf(stderr, "Failed to Fork Pawns PID#%d%s\n", getpid());
    		exit(EXIT_FAILURE);
    	case 0:
    		/* Giving the type */
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

    
    sem_reserve_1(master_sem_id, 0);
    /*fprintf(stderr, "a: %d, %d%s\n", a, errno, strerror(errno));*/
        
    while((select = wait(NULL)) != -1);

    /*printf("Risorsa liberata %d\n", getpid());*/
    
	exit(EXIT_SUCCESS);
}

void set_pawns(int letter, int parameters_id, int player_msg_id, int chessboard_mem_id, int chessboard_sem_id){
	static int more_player = 0;
	int i;
	int k;
	int l;
	struct message message_to_pawn;
	int test;
	struct param * parameters;

	int wall;

	int * chessboard;

	parameters = shmat(parameters_id,NULL,0);

    chessboard = shmat(chessboard_mem_id,NULL,0);

    if(letter == 69)
    	printf("DEFAULT----------------------\n");

	
	if(letter == 65){
		i = 0;
		l = 0;
	}else if(letter == 66){
		i = 0;
		l = parameters->SO_BASE-1;
	}else if(letter == 67){
		i = 0;
		l = (parameters->SO_BASE/4)-1;
		
		wall = l + l + l;
		printf("L per la C vale: %d, e WALL: %d\n", l, wall);
	}else if(letter = 68){
		i = parameters->SO_ALTEZZA-1;
		l = (parameters->SO_BASE/4)-1;
		
		wall = l + l + l;
		printf("L per la D vale: %d, e WALL: %d\n", l, wall);
	}else{
		printf("-----------------DEFAULT----------------------\n");
	}
	
	for(k = 0; k < parameters->SO_NUM_P; k++){
    	
		message_to_pawn.mtype = k+1;
		message_to_pawn.y = i;
		message_to_pawn.x = l;

		switch(letter){
			case 65:
			case 66:
				i++;
				if(i == parameters->SO_ALTEZZA){
					i = 0;
					if(letter == 65){
				    	l++;
					}else{
					    l--;
					}
				}

			break;

			case 67:
			case 68:
				l++;
				if(l == wall){
					l = (parameters->SO_BASE/4)-1;
					if(letter == 67){
						i++;
					}else{
						i--;
					}
				}

			break;

			default:
				while(1){
					message_to_pawn.y = rand() % parameters->SO_ALTEZZA;
					message_to_pawn.x = rand() % parameters->SO_BASE;
					if(chessboard[message_to_pawn.y * parameters->SO_BASE + message_to_pawn.x] == 1)
						break;
				}
			break;

		}
		
		chessboard[message_to_pawn.y * parameters->SO_BASE + message_to_pawn.x] = -letter;
		
		semctl(chessboard_sem_id, message_to_pawn.y * parameters->SO_BASE + message_to_pawn.x, SETVAL, 0);


			    /*message_to_pawn.strategy = "NSWEEWSN"*/

		test = msgsnd(player_msg_id, &message_to_pawn, sizeof(int) * 2, 0);
		if(test == -1){
			fprintf(stderr, "MSGSEND: ret: %d, errno: %d, %s\n", test, errno, strerror(errno));
		}	    
    }   
    
}