#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

char *command = NULL;
char *cmd = NULL, *parameters[20];  

void read_command() {
	command = readline("#");
	add_history(command);
	cmd = command;
	parameters[0] = cmd;

	int par_num = 1;
	
	for(size_t i = 0; i < strlen(command); i++){
		if(command[i] == '\0') break;
		if(command[i] == ' '){
			command[i] = '\0';
			if(command[i+1] != ' ') {
				parameters[par_num] = &command[i+1];
				par_num ++;
			}
		}
	}
}

int main(){
	while ( 1 ) {                               
		read_command();
		if (strcmp(cmd, "exit") == 0) {
			free(command);	
			break;
		}
		if ( fork() > 0 ) {        // 父进程 
			wait ( NULL );      
			cmd = NULL;
			for(int i = 0; i < 20; i++) {
				parameters[i] = NULL;
			}
			free(command);
		}
		else {
			execvp ( cmd, parameters);   // 执行子进程
		}
	}
	return 0;
}
