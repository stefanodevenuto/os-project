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

#define SO_FLAG_MIN 5		// 1. calcolo numero bandierine
#define SO_FLAG_MAX 40		// 2. calcolo punteggio bandierina sottrendo continuamente da SO_ROUND_SCORE
							// 3. vettore con posizioni libere e spawn random delle bandierine

#define SO_ROUND_SCORE 200
#define SO_ALTEZZA 20
#define SO_BASE 60

struct posizione{
	int row;
	int column;
};

int main(){

	int flag_number;
	int i;
	int j;
	struct posizione * pos;
	int chessboard_sem_id;
	int score;
	int total_score;


	total_score = SO_ROUND_SCORE;

	//chessboard_sem_id = semget(12345, 3 * 4, 0666 | IPC_CREAT);

	srand(getpid());

	if(SO_FLAG_MIN == SO_FLAG_MAX){
		flag_number = rand() % SO_FLAG_MAX + 1;
	}else{
		flag_number = rand() % (SO_FLAG_MAX - SO_FLAG_MIN + 1) + SO_FLAG_MIN;
	}

	/* Meglio creare un array di posizioni libere prima di spawnare posizioni random
	   in maniera tale da evitare chiamate di rand() inutili oppure contare sul fatto che sono
	   molte di più le celle libere rispetto a quelle piene ed evitare così una creazione di
	   un array e una "passata" sui semafori?*/

	/* Calcolo posizioni libere nella scacchiera 
	for(i=0; i < SO_ALTEZZA; i++){
		for(j=0; j < SO_BASE; j++){
			if(semctl(chessboard_sem_id, i * SO_BASE + j, GETVAL)){

			}

		}
	}*/


	int media;
	int sum = 0;
	/* ------------------------------------------------------------------------------ */
	int num;
	printf("number flag: %d\n", flag_number);
	//media = total_score/2;
	for(i = flag_number; i != 0 ; i--){
		media = total_score/i;
		num = rand() % media + 1;
		if()
		sum += num;
		total_score -= num;

		printf("%d --------> %d\n", num, media);
	}
		printf("%d --------> %d\n", total_score, media);
	printf("SOMMA: %d\n", sum + total_score);
	/* ------------------------------------------------------------------------------ */

}