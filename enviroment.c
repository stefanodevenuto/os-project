#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>



#ifdef HARD
    char path_num_g[11] ="SO_NUM_G=";
    char *input_num_g = "4";
    char path_num_p[11] ="SO_NUM_P=";
    char *input_num_p = "400";
    char path_max_time[14] ="SO_MAX_TIME=";
    char *input_max_time = "1";
    char path_base[10] ="SO_BASE=";
    char *input_base = "120";
    char path_altezza[13] ="SO_ALTEZZA=";
    char *input_altezza = "40";
    char path_flag_min[14] ="SO_FLAG_MIN=";
    char *input_flag_min = "5";
    char path_flag_max[14] ="SO_FLAG_MAX=";
    char *input_flag_max = "40";
    char path_round_score[17] ="SO_ROUND_SCORE=";
    char *input_round_score = "200";
    char path_n_moves[13] ="SO_N_MOVES=";
    char *input_n_moves = "200";
#else
    char path_num_g[11] ="SO_NUM_G=";
    char *input_num_g = "2";
    char path_num_p[11] ="SO_NUM_P=";
    char *input_num_p = "10";
    char path_max_time[14] ="SO_MAX_TIME=";
    char *input_max_time = "3";
    char path_base[10] ="SO_BASE=";
    char *input_base = "60";
    char path_altezza[13] ="SO_ALTEZZA=";
    char *input_altezza = "20";
    char path_flag_min[14] ="SO_FLAG_MIN=";
    char *input_flag_min = "5";
    char path_flag_max[14] ="SO_FLAG_MAX=";
    char *input_flag_max = "5";
    char path_round_score[17] ="SO_ROUND_SCORE=";
    char *input_round_score = "10";
    char path_n_moves[13] ="SO_N_MOVES=";
    char *input_n_moves = "20";
#endif

void set_env(){

    putenv(strcat(path_num_g, input_num_g));
    putenv(strcat(path_num_p, input_num_p));
    putenv(strcat(path_max_time, input_max_time));
    putenv(strcat(path_base, input_base));
    putenv(strcat(path_altezza, input_altezza));
    putenv(strcat(path_flag_min, input_flag_min));
    putenv(strcat(path_flag_max, input_flag_max));
    putenv(strcat(path_round_score, input_round_score));
    putenv(strcat(path_n_moves, input_n_moves));

}

int * get_env(){

    int * array = malloc(9 * sizeof(array));

    array[0] = atol(getenv("SO_NUM_G"));
    array[1] = atol(getenv("SO_NUM_P"));
    array[2] = atol(getenv("SO_MAX_TIME"));
    array[3] = atol(getenv("SO_BASE"));
    array[4] = atol(getenv("SO_ALTEZZA"));
    array[5] = atol(getenv("SO_FLAG_MIN"));
    array[6] = atol(getenv("SO_FLAG_MAX"));
    array[7] = atol(getenv("SO_ROUND_SCORE"));
    array[8] = atol(getenv("SO_N_MOVES"));

    return array;
}