#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#define SO_NUM_G 0
#define SO_NUM_P 1
#define SO_MAX_TIME 2
#define SO_BASE 3
#define SO_ALTEZZA 4
#define SO_FLAG_MIN 5
#define SO_FLAG_MAX 6
#define SO_ROUND_SCORE 7
#define SO_N_MOVES 8

void set_env();

int * get_env();