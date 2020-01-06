#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	int * a;

	a = (int *) malloc(sizeof(int) * 4);

	printf("sizeof a: %d\n", sizeof(a));

	a[0] = 3;


	return 0;
}