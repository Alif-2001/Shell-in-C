#include "shell.h"

/* split line according to '<' into an array of strings */
char **split_input_redirection(char *line);

/* split line according to '>' into an array of strings */
char **split_output_redirection(char *line);

/* check if line has '|' */
int is_pipe(char *line){
    if(strstr(line, "|") != NULL){
        return 1;
    }
    return 0;
}

/* check if line has '<' */
int is_input(char *line){
    if(strstr(line,"<")!= NULL){
        return 1;
    }
    return 0;
}

/* check if line has '>' */
int is_output(char *line){
    if(strstr(line,">")!=NULL){
        return 1;
    }
    return 0;
}

/* split line according to '|' into an array of strings */
char **split_pipe(char *line){
    char **tokens = malloc(BUFSIZE * sizeof(char*));
    char *token;

    token = strtok(line, "|");

    int i = 0;
    while(token != NULL) {
        tokens[i] = token;
        i++;
        token = strtok(NULL, "|");
    }
    tokens[i] = NULL;
    return tokens;
}

/* remove any extra space from the string */
void removeSpaces(char *str) 
{ 
    int c = 0; 
  
    for (int i = 0; str[i]; i++) {
        if (str[i] != ' ') {
            str[c++] = str[i];
        }
    }
    //make sure to end with NULL
    str[c] = '\0';
} 

/* split line according to '>' into an array of strings */
char **split_output_redirection(char *line){
    char **tokens = malloc(BUFSIZE * sizeof(char*));
    char *token;
    token = strtok(line, ">\n");

    int i = 0;
    while(token != NULL) {
        if(i == 1){
            removeSpaces(token);
        }
        tokens[i] = token;
        i++;
        token = strtok(NULL, ">\n");
    }
    tokens[i] = NULL;
    return tokens;
}

/* split line according to '<' into an array of strings */
char **split_input_redirection(char *line){
    char **tokens = malloc(BUFSIZE * sizeof(char*));
    char *token;
    token = strtok(line, "<\n");

    int i = 0;
    while(token != NULL) {
        if(i == 1){
            removeSpaces(token);
        }
        tokens[i] = token;
        i++;
        token = strtok(NULL, "<\n");
    }
    tokens[i] = NULL;
    return tokens;
}

/* create the pipe using pipe() and use dup2() to channel
    input and output.*/
int create_pipe(char *line) {
    pid_t childpid1;
    int status = 0;
    char **args = split_pipe(line);
    
    int fd[2];
    int store = 0;
    int i = 0;
    while(args[i]!=NULL){
        pipe(fd);
        childpid1 = fork();
        if(childpid1 >= 0){
            if(childpid1 == 0){
                if(args[i+1] == NULL && is_output(args[i])) {       // check if we have an output file
                    char **inp = split_output_redirection(args[i]);
                    freopen(inp[1],"w+",stdout);    //open the file in stdout, and print to that file
                    free(inp);
                }

                if(is_input(args[i]) && i == 0 ){       // check if we have an input file
                    char **inp = split_input_redirection(args[i]);
                    freopen(inp[1],"r",stdin);      //open the file in stdin, and take input from it
                    free(inp);
                }else{
                    dup2(store, STDIN_FILENO); // Otherwise take input from previous comand or stdin
                } 
                
                if(args[i+1] != NULL) {
                    dup2(fd[1],STDOUT_FILENO); // write to fd[1]
                }

                close(fd[0]); //unused
                status = exec_cmd(split_input(args[i]));        //execute the command by creating new process
                exit(status);
            }   
        }else{
            perror("fork");
            free(args);
            return (-1);
        }
        
        waitpid(childpid1, &status, 0); // wait for child to finish
        close(fd[1]); //unused
        store = fd[0]; // Used by child, send input for next command
        i++;
    }
    close(fd[0]);
    close(fd[1]);
    free(args);
    return 1;
}
