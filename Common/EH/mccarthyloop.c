/* Implementation of the McCarthy function in C */
#include <stdio.h>
#include <stdlib.h>

int mccarthy(int n) {
	if(n > 100)
		return n - 10;
	else
		return mccarthy(mccarthy(n+11));
}

int main(int argc, char *argv[]) {
	int n, i;
	if(argc != 2) {
		fprintf(stderr, "Usage: %s n\n", argv[0]);
		exit(1);
	}
	n = strtol(argv[1], NULL, 0);
	
	for(i = 0; i < n; i++)
		printf("%d\n", mccarthy(i));
	return 0;
}
