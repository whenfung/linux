#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


void printMaps(const char * name,  pid_t pid, const char * type){
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
	printMaps("分配内存前", pid, "/maps");
	
	char* p = NULL;
	int i = 128;
	while(NULL != (p = (char *)malloc(i*1024*1024))) {
		printf("%d\n", i);
		i = i+64;
		free(p);
	}
	printMaps("分配内存后", pid, "/status");
	return 0;
}
