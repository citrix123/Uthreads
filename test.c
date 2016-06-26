#include <stdio.h>
#include <stdlib.h>

struct t
{
	int *x;
};

int main()
{
	struct t *test;
	test = malloc(2 * sizeof(struct t));
	test[0].x = malloc(sizeof(int));
	*test[0].x = 8;
	printf("x = %d", *test[0].x);
	free(test[0].x);
	return 0;
}


