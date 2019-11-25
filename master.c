#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>

int main(int argc, char const *argv[]){
	
	char path_num_g[11] ="SO_NUM_G=";
    char *input_num_g = "2";
    putenv(strcat(path_num_g, input_num_g));

    char path_base[10] ="SO_BASE=";
	char *input_base = "3";
    putenv(strcat(path_base, input_base));

    char path_altezza[13] ="SO_ALTEZZA=";
    char *input_altezza = "5";
    putenv(strcat(path_altezza, input_altezza));

    

    int player_number;
    int chessboard_height;
    int chessboard_base;

    player_number = atol(getenv("SO_NUM_G"));
    chessboard_base = atoi(getenv("SO_BASE"));
    chessboard_height = atoi(getenv("SO_ALTEZZA"));

    
    char *chessboard = (char *)malloc(chessboard_base * chessboard_height * sizeof(int));
    //fprintf(stderr, "%d\n", errno );

    int i = 0;

    int j = 0;
    for(i=0;i<chessboard_base;i++){
    	for(j=0;j<chessboard_height;j++){
    		chessboard[ i * chessboard_height + j ] = '0';
    	}
    }

    for(i=0;i<chessboard_base;i++){
    	for(j=0;j<chessboard_height;j++){
    		printf("%c\t", chessboard[ i * chessboard_height + j ]);
    		
    		
    	}
    	
    	printf("\n");
    }

	
    /*


    Create Shared chessboard


    */

    /*printf("%d\n", player_number);
    printf("%d\n", chessboard_base);
    printf("%d\n", chessboard_height);*/
    
	return 0;
}