#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define N 256 * 1024 *1024

void print(const char * name,  pid_t pid, const char * type){
	printf("\n %s \n", name);
	char order[64] = "cat /proc/";
	char PID[8];
	sprintf(PID, "%d", pid);
	strcat(order, PID);
	strcat(order, type);
	system(order);
}

int main(){
	pid_t pid = getpid();
	print("分配内存前", pid, "/status");
	
	char* p = (char*) malloc(N);
	
	print("分配 256 M内存后", pid, "/status");
	
	int i;
	for(i = 0; i < N; i += 1024*4) {
		p[i] = i;
	}

	print("读操作之后", pid, "/status");
	return 0;
}
