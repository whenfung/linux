#include <stdio.h>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

// 
void type_prompt() {
	static int first_time = 1;
	if ( first_time ) {       // clear screen for the first time
		const char* CLEAR_SCREEN_ANSI = " \e[1;1H\e[2J";
		write(STDOUT_FILENO, CLEAR_SCREEN_ANSI,12);
		first_time = 0;
	}
	printf("#");              //display
}

void read_command ( char cmd[], char *par[] ) {
	char line[1024];
	int count = 0, i = 0, j = 0;
	char *array[100], *pch;

	// Read one line
	for(;;) {
		int c = fgetc ( stdin );
		line[count++] = (char) c;
		if ( c == '\n') break;
	}
	if ( count == 1 ) return;
	pch = strtok ( line, " \n");

	// parse the line into words
	while ( pch != NULL ) {
		array[i++] = strdup ( pch );
		pch = strtok ( NULL, " \n");
	}  
	// first word is the command
	strcpy ( cmd, array[0] );

	// others are parameters
	for ( j = 0; j < i; j++ )
		par[j] = array[j];
	par[i] = NULL; //NULL-terminate the parameter list
}

int main(){
	char cmd[100], command[100], *parameters[20];  
	// environment variable 
	char *envp[] = { (char *) "PATH=/bin", 0 };
	while ( 1 ) {         // repeat forever
		type_prompt();    // display prompt on screen
		read_command ( command, parameters ); // read input from terminal
		if ( fork() != 0 ) //parent
			wait ( NULL ); //wait for child
		else {
			strcpy ( cmd, "/bin/" );
			strcat ( cmd, command );
			execve ( cmd, parameters, envp ); //execute command 
		}
		if ( strcmp ( command, "exit" ) == 0 )
			break;
	}
	return 0;
}
