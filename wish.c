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
pid_t launch_command(char **);
void execute_command(char **);


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
        write(STDERR_FILENO, error_message, strlen(error_message));
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

    	int path_c = 0;
   	while(args[path_c + 1] != NULL) {
        	path_c++;
    	}

    	if (path_c == 0) { 
        	path_cnt = 0;
        	path_dirs = NULL;
        	return;
    	}

	path_dirs = malloc((path_c + 1) * sizeof(char *));
	if (path_dirs == NULL){
		Error();
        	exit(1);
	}

	for(int i = 0; i < path_c; i++){
		path_dirs[i] = strdup(args[i + 1]);
		if (path_dirs[i] == NULL){
			Error();
            		exit(1);
		}
	}
	path_dirs[path_c] = NULL;
	path_cnt = path_c;
}


// parse the input into and array of tokens
char **parsing(char *line){
	int bufsize = 1;
	char ** ret = malloc(bufsize * sizeof(char*));
	if (ret == NULL) {
		Error();
        exit(1);
    }
	char *token;
    char *saveptr;

	int idx = 0;
	token = strtok_r(line, " \t\n\r", &saveptr);
	while(token != NULL){
		ret[idx++] = token;
		if (idx == bufsize){
			bufsize++;
			ret = realloc(ret, bufsize * sizeof(char*));
			if (ret == NULL) {
				Error();
                exit(1);
            }
		}
		token = strtok_r(NULL, " \t\n\r", &saveptr);
	}
	ret[idx] = NULL;
	return ret;
}

bool builtin_command(char ** args){
	if (args[0] == NULL) return false; // Handle empty command
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
	// Must be exactly one argument after '>' and it must be the last one
	return args[pos] != NULL && args[pos + 1] == NULL;
}


void execute_command(char **args){
	if (args[0] == NULL)return;
	for(int i = 0; i < path_cnt; i++){
		int len = strlen(path_dirs[i]) + strlen(args[0]) + 2;
	        char *full_path = malloc(len * sizeof(char));
          	if (full_path == NULL) {
                	Error();
                	exit(1);
            	}

	        strcpy(full_path, path_dirs[i]);
       	 	strcat(full_path, "/");
       		strcat(full_path, args[0]);

		if (!access(full_path, X_OK)){
			execv(full_path, args);
			free(full_path);
			Error();
			exit(1);
		}
		free(full_path);
	}
	Error();
}

void Free_Redirection(char ** args, int pos){
	args[pos] = NULL;
}

pid_t launch_command(char **args){

	if (args[0] == NULL){
		return 0;
	}

	if (builtin_command(args)){
		return 0; // Built-in command executed, return 0 (no PID)
	}
 	else {
		pid_t ch = fork();
		if (!ch){ // Child Process
			int pos = check_redirection(args);
			if (pos == 0){ // Error: multiple redirections
				Error();
			}
			else if (pos > 0){ // Redirection found
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
						if (dup2(fd, 2) < 0) { 
							Error();
						}
						
						close(fd);
						Free_Redirection(args, pos);
						execute_command(args);
						
					}
				}
				else Error(); // Invalid redirection (e.g., "ls > out1 out2" or "ls >")
			}
			else{ // No redirection
				execute_command(args);
			}
			exit(1);
		}
		else if (ch > 0){
			return ch;
		}
		else {
			Error();
			return -1;
		}
	}
}

char *preprocess_redirection(char *str){
    int count = 0;
    // Count how many '>' characters we have
    for(int i = 0; str[i] != '\0'; i++){
        if (str[i] == '>') count++;
    }

    // No redirection, just return a copy of the original string
    if (count == 0) {
        return strdup(str);
    }

    // Each ">" will be replaced with " > " (adding 2 spaces)
    char *new_str = malloc(strlen(str) + 2 * count + 1);
    if (new_str == NULL) {
        Error();
        exit(1);
    }

    char *writer = new_str;
    char *reader = str;

    while (*reader != '\0'){
        if (*reader == '>'){
            *writer++ = ' ';
            *writer++ = '>';
            *writer++ = ' ';
        } else {
            *writer++ = *reader;
        }
        reader++;
    }
    *writer = '\0';
    return new_str;
}
void loop(FILE *stream, bool interactive){

	char *line = NULL;
    	ssize_t nread;
    	size_t len;
	char **args;

	pid_t *pids = NULL; 
	int pid_count = 0;
	int pid_capacity = 0;

	char *saveptr_loop; //Save pointer for this loop's strtok_r

	while(true){
		if (interactive){
			printf("wish> ");
			fflush(stdout);
		}

		nread = getline(&line,&len, stream);
		if (nread == -1) {
            		break;
            	}
        

		pid_count = 0;

		char *command_str = strtok_r(line, "&\n", &saveptr_loop);

		while(command_str != NULL){
            		char *processed_str = preprocess_redirection(command_str);
			args = parsing(processed_str); 

			if (args[0] != NULL) { 
				if (pid_count >= pid_capacity){
					pid_capacity = (pid_capacity == 0) ? 8 : pid_capacity * 2;
					pids = realloc(pids, pid_capacity * sizeof(pid_t));
					if (pids == NULL) {
						Error();
						exit(1);
					}
				}

				pid_t pid = launch_command(args);

				if (pid > 0){ 
					pids[pid_count++] = pid;
				}
			}

            		free(processed_str); 
			free(args); 

			command_str = strtok_r(NULL, "&\n", &saveptr_loop); 
		}

		for (int i = 0; i < pid_count; i++){
			int status;
			waitpid(pids[i], &status, 0);
		}

		free(line);
		line = NULL;
	}
	free(pids); 
}

int main(int argc, char *argv[]){
	// Set initial path to /bin
	char *initial_path_args[] = {"path", "/bin", NULL};
	Path(initial_path_args);


	if (argc > 2){
		Error();
		exit(1);
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
 	
	free_paths(); 
	return 0;
}
