#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#define NUM_G 2
#define NUM_P 10
#define MAX_TIME 3
#define BASE 60
#define ALTEZZA 20
#define FLAG_MIN 5
#define FLAG_MAX 5
#define ROUND_SCORE 10
#define N_MOVES 20
#define MIN_HOLD_NSEC 100000000

#define SO_NUM_G 0
#define SO_NUM_P 1
#define SO_MAX_TIME 2
#define SO_BASE 3
#define SO_ALTEZZA 4
#define SO_FLAG_MIN 5
#define SO_FLAG_MAX 6
#define SO_ROUND_SCORE 7
#define SO_N_MOVES 8
#define SO_MIN_HOLD_NSEC 9

/*#define SO_NUM_G 4
#define SO_NUM_P 400
#define SO_MAX_TIME 1
#define SO_BASE 120
#define SO_ALTEZZA 40
#define SO_FLAG_MIN 5
#define SO_FLAG_MAX 40
#define SO_ROUND_SCORE 200
#define SO_N_MOVES 200
#define SO_MIN_HOLD_NSEC 100000000*/

#ifdef HARD
	#define MEM_KEY 12346
#else
	#define MEM_KEY 12345
#endif

void set_env();

int * get_env();