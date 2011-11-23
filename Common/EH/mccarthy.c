/* Implementation of the McCarthy function in C */
#include <stdio.h>
#include <stdlib.h>

int mccarthy(int n) {
	if(n > 100)
		return n - 10;
	else
		return mccarthy(mccarthy(n+11));
}

int main(void) {
	char ns[4];
	char *ns2 = fgets(ns, 3, stdin);
	if(ns == NULL) {
		exit(1);
	}
	int n = strtol(ns, NULL, 3);
	printf("%d\n", mccarthy(n));
	return 0;
}