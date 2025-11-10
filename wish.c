#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>

//Functions Declarations
void Error();
void Cd(char **);
void Path(char **);
void Exit(char **);
char **parsing(char *);


char *builtin_cmd[] = {"cd", "exit", "path"};
void (*builtin_fun[])(char **) = {
	&Cd,
	&Exit,
	&Path
};

// To make getting the size easier in the future
int builtins_no() {
    return sizeof(builtin_cmd) / sizeof(char *);
}

void Error(){
	char error_message[30] = "An error has occurred\n";
        write(1, error_message, strlen(error_message));
}

void Exit(char ** args){
	if (args[1] != NULL) Error();  // Check for extra arguments after "exit"
	printf("See You Later\n");
	exit(0);
}
void Cd(char **args){
	// I just follow the requirements of the project, it doesn't necesserally to be right in the real world
	if (args[1] == NULL || args[2] != NULL || chdir(args[1]) == -1){
		Error();
	}
}

void Path(char **args){}

// parse the input into and array of tokens
char **parsing(char *line){
	int bufsize = 1;
	char ** ret = malloc(bufsize * sizeof(char*));
	if (ret == NULL)
		Error();
	char *token;
	int idx = 0;
	token = strtok(line, " \t\n\r");
	while(token != NULL){
		ret[idx++] = token;
		if (idx == bufsize){
			bufsize++;
			ret = realloc(ret, bufsize * sizeof(char*));
			if (ret == NULL)
				Error();
		}
		token = strtok(NULL, " \t\n\r");
	}
	ret[idx] = NULL;
	return ret;
}

bool builtin_command(char ** args){
	for(int i = 0; i < builtins_no(); i++){
		if (!strcmp(args[0], builtin_cmd[i])){
			(*builtin_fun[i])(args);
			return true;
		}
	}
	return false;

}

bool execute(char **args){
	if (builtin_command(args){
		return 1;
	}
 	else {
		pid_t ch = fork();
		if (!ch){
			if (builtin_command(args)){
				return 1;
			}
			exit(0);
		}
		else if (ch > 0){
			wait(NULL);
			return true;
		}
		else Error();
	}
}
void loop(FILE *stream, bool interactive){

	char *line = NULL;
        ssize_t nread;
        size_t len;
	char **args;
	int status = 1;
	while(status){
		if (interactive){
			printf("wish> ");
			fflush(stdout);
		}

		nread = getline(&line,&len, stdin);

		args = parsing(line);

		status = execute(args);

		// free memory
		free(line);
		line = NULL;
		free(args);
	}
}

int main(int argc, char *argv[]){
	if (argc > 2){
		Error();
	}else if (argc == 2){

	}else{
		loop(stdin, true);
		exit(1);
		while(true){
			printf("wish> ");
			char *line = NULL;
			ssize_t nread;
			size_t len;
			nread = getline(&line, &len, stdin);
			pid_t ch = fork();
			if (!ch){
				printf("I'm the child HEEEY !!\n");
				exit(0);
			}
			else if (ch > 1){
				wait(NULL);
				printf("I'm the parent be quite\n");
			}
			else {
				printf("Nothing Happend\n");
			}
		}
	}
 	
	return 0;
}
