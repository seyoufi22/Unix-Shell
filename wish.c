#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>

//Functions Declarations

void Error();
bool Exit();
char **parsing(char *);

char *builtin_cmd[] = {"cd", "exit", "path"};
int builtins_no() {
    return sizeof(builtin_cmd) / sizeof(char *);
}
void Error(){
	char error_message[30] = "An error has occurred\n";
        write(1, error_message, strlen(error_message));
	exit(1); 
}

void Exit(){
	printf("See You Later\n");
	exit(0);
}



// parse the input into and array of tokens
char **parsing(char *line){
	int bufsize = 64;
	char ** ret = malloc(bufsize * sizeof(char*));
	if (ret == NULL)
		Error();
	char *token;
	int idx = 0;
	token = strtok(line, " ");
	while(word != NULL){
		ret[idx++] = token;
		if (idx == bufsize){
			bufsize += 64;
			ret = realloc(ret, bufsize * sizeof(char*));
			if (ret == NULL)
				Error();
		}
		token = strtok(NULL, " ");
	}
	ret[idx] = NULL;
	return ret;
}

void loop(FILE *stream, bool interactive){

	char *line = NULL;
        ssize_t nread;
        size_t len;
	char **argv;
	int status = 1;
	while(status){
		if (interactive){
			printf("wish> ");
			fflush(stdout);
		}

		nread = getline(&line,&len, stdin);

		argv = parsing(line);

	//	status = execute(argv);   TBD

		// free memory
		free(line);
		line = NULL;
		free(argv);
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
			if (Exit(line)){
				printf("See You Later\n");
				exit(0);
			}
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
