#include <stdio.h>
#include <fcntl.h>
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
void execute(char **);

int path_cnt = 0;
char **path_dirs;
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
        write(2, error_message, strlen(error_message));
}

void Exit(char ** args){
	if (args[1] != NULL){
		Error();  // Check for extra arguments after "exit"
		return;
	}
	exit(0);
}
void Cd(char **args){
	// I just follow the requirements of the project, it doesn't necesserally to be right in the real world
	if (args[1] == NULL || args[2] != NULL || chdir(args[1]) == -1){
		Error();
	}
}

void free_paths(){
	if (path_dirs == NULL)return;
	for(int i = 0; i < path_cnt; i++){
		free(path_dirs[i]);
	}
	free(path_dirs);
	path_dirs = NULL;
	path_cnt = 0;
}

void Path(char **args){
	free_paths();
	if (args[1] == NULL)return;
	int path_c = 0;
	while(args[++path_c] != NULL)
	path_dirs = malloc(path_c * sizeof(char *));
	if (path_dirs == NULL){
		Error();
	}
	for(int i = 0; i < path_c - 1; i++){
		path_dirs[i] = strdup(args[i + 1]);
		if (path_dirs[i] == NULL){
			Error();
		}
	}
	path_dirs[path_c - 1] = NULL;
	path_cnt = path_c - 1;
}



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


int check_redirection(char ** args){
	int c = 0, pos = -1;
	for(int i = 0; args[i] != NULL; i++){
		if (!strcmp(args[i], ">")){
			c++;
			pos = i;
		}
	}
	if (c > 1)pos = 0;
	return pos;
}

bool valid_redirection(char ** args, int pos){
	return args[pos] != NULL && args[pos + 1] == NULL;
}

void execute_command(char **args){
	if (args[0] == NULL)return;
	for(int i = 0; i < path_cnt; i++){
		int len = strlen(path_dirs[i]) + strlen(args[0]) + 2;
	        char *full_path = malloc(len * sizeof(char));

	        strcpy(full_path, path_dirs[i]);
       	 	strcat(full_path, "/");
       		strcat(full_path, args[0]);

		if (!access(full_path, X_OK)){
			execv(full_path, args);
		}
		free(full_path);
	}
	Error();
}

void Free_Redirection(char ** args, int pos){
	args[pos] = NULL;
}

void execute(char **args){
	if (builtin_command(args)){
		return;
	}
 	else {
		pid_t ch = fork();
		if (!ch){
			int pos = check_redirection(args);
			if (!pos){
				printf("HDF\n");
				Error();
			}
			else if (~pos){
				if (valid_redirection(args, pos + 1)){
					char *output_file = args[pos + 1];
					int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
					if (fd < 0){
						Error();
					}
					else {
						if (dup2(fd, 1) < 0) {
            						Error();
        					}
						else {
							close(fd);
							Free_Redirection(args, pos);
							execute_command(args);
						}
					}
				}
				else Error();
			}
			else{
				execute_command(args);
			}
			exit(0);
		}
		else if (ch > 0){
			wait(NULL);
		}
		else Error();
	}
}



void loop(FILE *stream, bool interactive){

	char *line = NULL;
        ssize_t nread;
        size_t len;
	char **args;
	while(true){
		if (interactive){
			printf("wish> ");
			fflush(stdout);
		}

		nread = getline(&line,&len, stream);
		if (nread == -1) {
                    break; // Exit the loop
                }
		args = parsing(line);
		
		execute(args);
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
		FILE *fp = fopen(argv[1], "r");
		if (fp == NULL){
			Error();
			exit(1);
		}
		loop(fp, false);
		fclose(fp);
	}else{
		loop(stdin, true);
	}
 	
	return 0;
}
