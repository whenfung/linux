#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define N 7

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
	print("分配内存前", pid, "/maps");
	
	char* p[N];
	int i;
	for(i = 0; i < 6; i++) { 
		if(NULL ==  (p[i] = (char *)malloc(128 * 1024 * 1024))) {
			perror("malloc error ...");
			exit(1);
		}
		print("分配 1~6 号内存", pid, "/maps");
	}
	free(p[2]);
	print("释放 2",pid, "/maps");
	free(p[3]);
	print("释放 3", pid, "/maps");
	free(p[5]);
	print("释放 4", pid, "/maps");

	if(NULL == (p[6] = (char *)malloc(1024 * 1024 * 1024))) {
		perror("malloc error ...");
		exit(1);
	}
	print("分配 1024 MB", pid, "/maps");


	p[2] = (char *)malloc(64 * 1024 * 1024);
	print("再次分配 64 MB 内存", pid, "/maps");
	free(p[2]);
	
	free(p[0]);
	free(p[1]);
	free(p[4]);
	free(p[6]);

	return 0;
}
