#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define N 7

void printMaps(const char * name,  pid_t pid){
	printf("\n %s \n", name);
	char order[64] = "cat /proc/";
	char PID[8];
	sprintf(PID, "%d", pid);
	strcat(order, PID);
	strcat(order, "/maps");
	system(order);
}

int main(){
	pid_t pid = getpid();
	printMaps("分配内存前", pid);
	
	char* p[N];
	int i;
	for(i = 0; i < 6; i++) { 
		if(NULL ==  (p[i] = (char *)malloc(128 * 1024 * 1024))) {
			perror("malloc error ...");
			exit(1);
		}
		printMaps("分配 1~6 号内存", pid);
	}
	free(p[2]);
	printMaps("释放 2",pid);
	free(p[3]);
	printMaps("释放 3", pid);
	free(p[5]);
	printMaps("释放 4", pid);

	if(NULL == (p[6] = (char *)malloc(1024 * 1024 * 1024))) {
		perror("malloc error ...");
		exit(1);
	}
	printMaps("分配 1024 MB", pid);


	p[2] = (char *)malloc(64 * 1024 * 1024);
	printMaps("再次分配 64 MB 内存", pid);
	free(p[2]);
	
	free(p[0]);
	free(p[1]);
	free(p[4]);
	free(p[6]);

	return 0;
}
