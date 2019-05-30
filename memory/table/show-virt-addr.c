#include <stdio.h>

int main(){
	const char* str = "Hello world!\n";
	printf("%s @ %p\n", str, str);
	getchar();
	return 0;
}
