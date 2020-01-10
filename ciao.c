#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct flag{
	int number;
	int position;
	int x;
	int y;
	int distance;
	int checked;
};

int main(int argc, char const *argv[])
{
	int i;
	struct flag * a;

	a = (struct flag *) malloc(4 * sizeof(struct flag));

	memset(a, 0, 4 * sizeof(struct flag)); 

	for(i = 0; i < 4; i++){
		printf("%d", a[i].number);
		printf("%d", a[i].position);
		printf("%d", a[i].x);
		printf("%d", a[i].y);
		printf("%d", a[i].distance);
		printf("%d", a[i].checked);
	}

	


	return 0;
}