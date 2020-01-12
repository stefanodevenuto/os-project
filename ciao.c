#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
	int * a;
}PAWN;

int miao(PAWN * pawn);

int main(int argc, char const *argv[])
{
	PAWN pawn;

	printf("CIAo\n");
	
	pawn.a = malloc(sizeof(int) * 4);
	
	miao(&pawn);

	printf("%d\n", pawn.a[0]);


	return 0;
}

int miao(PAWN * pawn){
	printf("MIAO\n");
	pawn->a[0] = 4;
}