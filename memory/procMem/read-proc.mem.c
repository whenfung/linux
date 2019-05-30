#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<string.h>
#include<sys/ptrace.h>

int main(int argc, char **argv)
{
	if(argc != 3) exit(-1);
	
	char filename[200];
	char buf[200];
	bzero(filename,200);  // 置零
	bzero(buf,200);

	sprintf(filename,"/proc/%s/mem",argv[1]); // 获取进程号对应的 mem
	printf("filename: %s\n",filename);        
	int fd = open(filename, O_RDONLY);         // 打开 mem  
	printf("fd: %d\n", fd);                   // 输出文件描述符

	int pid;
	sscanf(argv[1], "%d", &pid);              // 获取进程号 
	ptrace(PTRACE_ATTACH, pid, 0, 0);         // 利用 ptrace 查看其他进程的内存空间
	
	//根据命令参数确定/proc/PID/mem 文件名
	//打开/proc/PID/mem
	//ptrace 跟踪目标进程
	
	long offset;
	sscanf(argv[2],"%lx", &offset);
	off_t r = lseek(fd, offset, SEEK_SET);  //其中的定位使用 64 位定位方式	
	
	if(r==-1) printf("lseek error:%m\n");
	printf("位置: %lx\n",r);

	ssize_t size = read(fd, buf, 200);
	//读入目标进程的/proc/PID/mem 内容
	if(size==-1) printf("read error:%m\n");
	printf("Data at offset %lx: %s\n", offset, buf); //输出目标进程指定地址上的内容
	close(fd);
	return 0;
}
