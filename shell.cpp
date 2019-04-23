#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
 
void type_prompt() {
	static int first_time = 1;
	if ( first_time ) {       // 清除屏幕
		const char* CLEAR_SCREEN_ANSI = " \e[1;1H\e[2J";
		write(STDOUT_FILENO, CLEAR_SCREEN_ANSI,12);
		first_time = 0;
	}
	printf("#");              // 显示
}

void read_command ( char cmd[], char *par[] ) {
	char line[1024];
	int count = 0, i = 0, j = 0;
	char *array[100], *pch;

	// 读一行字符
	for(;;) {
		int c = fgetc ( stdin );
		line[count++] = (char) c;
		if ( c == '\n') break;
	}
	if ( count == 1 ) return;
	pch = strtok ( line, " \n");

	// 将一行解析为单词
	while ( pch != NULL ) {
		array[i++] = strdup ( pch );
		pch = strtok ( NULL, " \n");
	}  
	// 第一个单词就是指令
	strcpy ( cmd, array[0] );

	// 其他单词就是参数
	for ( j = 0; j < i; j++ )
		par[j] = array[j];
	par[i] = NULL; // NULL 表示参数的结束
}

int main(){
	// 一个命令包括命令和参数
	char cmd[100], command[100], *parameters[20];  
	// 环境变量，调用可执行程序
	char *envp[] = { (char *) "PATH=/bin", 0 };
	while ( 1 ) {                               // 进入是循环
		type_prompt();                          // 显示指令
		read_command ( command, parameters );   // 从终端读取指令
		if ( fork() != 0 )                      // 父进程阻塞
			wait ( NULL );                      // 等待子进程
		else {
			strcpy ( cmd, "/bin/" );            // 环境变量 
			strcat ( cmd, command );            // 拼接程序路径
			execve ( cmd, parameters, envp );   // 执行命令
		}
		if ( strcmp ( command, "exit" ) == 0 )  // 如果第一个单词是 exit ，那就退出程序 
			break;
		if ( strcmp ( command, "clear" ) == 0 ) // 我喜欢 clear
			system("echo hello world!");			
	}
	return 0;
}
