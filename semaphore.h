#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
                       /*       ________ _________ ___ ____________  */
#define MAIN_SEM 55553 /* ---> | MASTER | SYNCHRO | A | TURN_ENTRY | */
#define MASTER 0       /*       -------- --------- --- ------------  */  
#define SYNCHRO 1
#define A 2
#define TURN_ENTRY 3
#define CHESSBOARD_MEM_KEY 44445
#define CHESSBOARD_SEM_KEY 33334
#define PARAMETERS_MEM_KEY 22223
#define LEN_X_Y 8
#define STRAT_LEN 300


struct param{
    int SO_NUM_G;
    int SO_NUM_P;
    int SO_MAX_TIME;
    int SO_BASE;
    int SO_ALTEZZA;
    int SO_FLAG_MIN;
    int SO_FLAG_MAX;
    int SO_ROUND_SCORE;
    int SO_N_MOVES;
    int SO_MIN_HOLD_NSEC;
};

struct message{
	long mtype;
	int x;
	int y;
	/*char * strategy;*/
};

struct strategy{
    long mtype;
    char strategy[STRAT_LEN]; 
};
/*#define POS(row,column) ((row)*(COLUMNS)+(column))*/


/*
 * The following union must be defined as required by the semctl man
 * page
 */
/*
union semun {
	int              val;    
	struct semid_ds *buf;    
	unsigned short  *array;  
	struct seminfo  *__buf;  
				    
};


/*
 * Set a semaphore to a user defined value
 * INPUT:
 * - sem_id: the ID of the semaphore IPC object
 * - sem_num: the position of the semaphore in the array
 * - sem_val: the initialization value of the semaphore
 * RESULT:
 * - the selected semaphore is initialized to the given value
 * - the returned value is the same as the invoked semctl
 */
int sem_set_val(int sem_id, int sem_num, int sem_val);

/*
 * Try to access the resource
 * INPUT:
 * - sem_id: the ID of the semaphore IPC object
 * - sem_num: the position of the semaphore in the array
 * RESULT
 * - if the resource is available (semaphore value > 0), the semaphore
 *   is decremented by one
 * - if the resource is not available (semaphore value == 0), the
 *   process is blocked until the resource becomes available again
 * - the returned value is the same as the invoked semop
 */
int sem_reserve_1(int sem_id, int sem_num);

/*
 * Try to access the resource
 * INPUT:
 * - sem_id: the ID of the semaphore IPC object
 * - sem_num: the position of the semaphore in the array
 * RESULT
 * - if the resource is available (semaphore value == 0), the waiting
 *   process go
 * - if the resource is not available (semaphore value > 0), the
 *   process is blocked until the resource becomes available again
 * - the returned value is the same as the invoked semop
 */
int sem_reserve_0(int sem_id, int sem_num);

/*
 * Release the resource
 * INPUT:
 * - sem_id: the ID of the semaphore IPC object
 * - sem_num: the position of the semaphore in the array
 * RESULT:
 * - the semaphore value is incremented by one. This may unblock some
 *   process
 * - the returned value is the same as the invoked semop
 */
int sem_release(int sem_id, int sem_num);

/*
 * Print all semaphore values to a string. my_string MUST be
 * previously allocated
 */
