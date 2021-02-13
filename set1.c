#include "shell.h"

//A list of built-in commands
char *internals[] = {
    //"kill",
    "exit",
    "cd",
    "echo",
    "export",
    "history"
};


/* This function launches the shell loop */
void launch_loop(){
    head = mmap(NULL, sizeof *head, PROT_READ | PROT_WRITE, 
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    char **args;
    char *line;
    int status = 1;
    head = NULL;
    char cwd[BUFSIZE];

    initialize_shell();

    while(status){
        printf("%s > ", getcwd(cwd, BUFSIZE));
        line = read_input(); //read user input
        save_cmd(line); //save the comands in the history file
        if(is_pipe(line) || is_input(line) || is_output(line)) {
            create_pipe(line); //if line has '|','<' or '>' then pipe.
        }else{
            args = split_input(line); //if not then split input into args
            status = create_fork(args); //and create a fork to exec the comand
            free(args);
        }
        free(line);
    }
    free_env_var(); //free malloced environment vars
}

/* for sigact structure when the process is killed */ //(does not work)
void sigquit(int signo) {
    printf("[%d]+ done\n",getpid());
    exit(0);
}

/* fork using fork() and run comand in child using execvp() */
int create_fork(char **args){

    if(args[0] == NULL) {       //check if there are any arguments.
        return 1;
    }

    if(is_cmd_internal(args)){      
        return exec_internal_cmd(args);   //check if command is built-in and run it.  
    }
    
    struct sigaction sigact;    //setup sigaction structure
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_handler = sigquit;

    if (sigaction(SIGQUIT, &sigact, NULL) < 0) {
        perror("sigaction()");
        return -1;
    }

    pid_t childpid;
    int status = 0;
    int bg = is_cmd_background(args);       //check if comand has '&' at the end
    childpid = fork();      //create fork

    if ( childpid >= 0 ) {
        if (childpid == 0) {        //child process
            status = exec_cmd(args);    //execute the comand
            if(status == -1){
                printf("Comand not found\n");
            }
            exit(status);
        } else {        //parent process
            if(bg == 1){        //if process is a background process
                push(args, childpid);   //record command in the list of background jobs
                sigact.sa_handler = SIG_DFL;
                sigaction(SIGQUIT,&sigact,NULL);
                waitpid(childpid,&status,WNOHANG);
                printf("[%d] %d\n", head->position, childpid);
                
            }else{
                waitpid(childpid,&status,0);    //wait for the child process to finish
            }
        }
    } else {
        perror("fork");
        return(-1);
    }

    kill_completed_background(&status); //keep checking for completed background jobs

    return 1;
}

/* read input from command line */
char *read_input(){
    char *line = NULL;
    size_t length = 0;
    ssize_t nread = 0;
    line = (char *)malloc(256);
    nread = getline(&line, &length, stdin);
    return line;
}

/* split the input into an array of strings */
char **split_input(char *line){
    char **tokens = malloc(BUFSIZE * sizeof(char*));
    char *token;
    //split accorind to spaces, tabs...
    token = strtok(line, " \"\t\n\a\r"); //remove spaces, tabs...

    int i = 0;
    while(token != NULL) {
        tokens[i] = token;
        i++;
        token = strtok(NULL, " \"\t\n\a\r");
    }
    tokens[i] = NULL;
    return tokens;
}

/* check if command is in the list of built-ins */
int is_cmd_internal(char **args) {
    int numOfCmd = sizeof(internals) / sizeof(char *);

    for (int i = 0; i < numOfCmd ; i++){
        if(strcmp(args[0], internals[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* check if the command has '&' for background processes */
int is_cmd_background(char **args) {
    int count = 0;
    while (args[++count] != NULL);
    if (strcmp(args[count-1], "&") == 0) {      //check if last letter is '&'
        args[count-1] = NULL;
        return 1;
    }
    return 0;
}

/* call built-in functions */
int exec_internal_cmd(char **args) {
    if(strcmp(args[0],"kill") == 0 || (strcmp(args[0], "exit")) == 0){
        return sh_exit(args);
    }

    if(strcmp(args[0],"cd") == 0){
        return sh_cd(args);
    }

    if(strcmp(args[0],"export") == 0){
        return sh_export(args);
    }

    if(strcmp(args[0],"echo") == 0){
        return sh_echo(args);
    }

    if(strcmp(args[0],"history") == 0){
        return sh_history(args);
    }

    return 1;
}

/* use execvp() to execute the commands
and start new process */
int exec_cmd(char **args) {
    int status = -1;
    status = execvp(args[0],args);
    return status;
}

/* Print all the commands in args */
void print_commands(char **args){
    int i = 0;
    while(args[i]!=NULL){
        printf(" %s", args[i]);
        i++;
    }
    printf("\n");
}

/* checks for background processes and kill if
    they are done executing */
void kill_completed_background(int* status) {
    if (head != NULL) {
        struct Background *fakeHead = head;
        while(fakeHead != NULL){    //iterate through all the background processes
            int check = waitpid(fakeHead->id,status,WNOHANG);   //check if process has finished executing
            if(check != 0){
                int position = fakeHead->position;
                kill(fakeHead->id,SIGQUIT);     //send a kill signal
                printf("[%d]+ Done          ", position);
                print_commands(fakeHead->cmd);
                deleteNode(position);       //remove it from list of background commands
            }
            fakeHead = fakeHead->next;
        }
    }
}

/* builtin exit command */
int sh_exit(char **args) {
    printf("Exiting Shell\n");
    struct Background * node = head;
    while(node != NULL){
        printf("Killing background process [%d]        ", node->position);
        print_commands(node->cmd);
        kill(node->id,SIGQUIT);     //kill any active background processes
        deleteNode(node->position);     //delete it from the list of background processes
        node = node->next;
    }
    return 0;
}

/* helps push a new process to list of background processes */
void push(char **args, pid_t newId) {
    struct Background * node;
    if(!(node = malloc(sizeof(struct Background)))){
        exit(0);
    }

    node->cmd = malloc(sizeof(char*));
    int i = 0;
    while(args[i]!=NULL){
        node->cmd[i] = malloc(BUFSIZE);
        strcpy(node->cmd[i], args[i]);      //store arguments
        //node->cmd[i] = args[i];
        i++;
    }
    //node->cmd[i] = NULL;
    
    node->id = newId;       //store the PID
    if(head != NULL){
        int position = head->position;
        position++;     //Set its position
        node->position = position;
    }else{
        node->position = 1;
    }

    node->next = head;
    head = node;
}

/* helps delete a process from the list of background processes */
void deleteNode(int position) {     //deletes node with thhe given position
    if(head != NULL){
        if(head->position == position) {
            struct Background* node = head;
            head = node->next;
            int i = 0;
            while(node->cmd[i] != NULL){
                free(node->cmd[i]);
                i++;
            }
            free(node->cmd);
            free(node);           
        }else{
            struct Background* fakeHead = head;
        
            while(fakeHead->next->position != position && fakeHead->next != NULL){
                fakeHead = fakeHead->next;
            }

            struct Background* node = fakeHead->next;
            fakeHead->next = node->next;
            free(node->cmd);
            free(node);
        }
    }
}
