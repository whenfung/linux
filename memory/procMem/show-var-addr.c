#include <stdio.h>

int main(){
	char Mesg[] = "Hello World!\n";
	printf("Mesg's addr: %lx\n", (long)Mesg);
	getchar();
	return 0;
}
