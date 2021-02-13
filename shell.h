#ifndef _SHELL_H
#define _SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h> 
#include <fcntl.h>

#define BUFSIZE 128

struct Background{
    char **cmd;
    pid_t id;
    int position;
    struct Background* next;
};

struct envVariable{     //add anything else?
    char *path;
};

static struct Background *head;
static struct envVariable Path;
static struct envVariable Home;
static struct envVariable Hist;

//Set1
void launch_loop();
int create_fork(char **args);
char *read_input();
char **split_input(char *line);

int exec_cmd(char **args);
int is_cmd_internal(char **args);
int exec_internal_cmd(char **args);
int is_cmd_background(char **args);

void kill_completed_background(int* status);

//buitlin + set3
int sh_exit(char **args);
int sh_cd(char **args);
int sh_export(char **args);
int sh_echo(char **args);
int sh_history(char **args);

//Background Linked_list
void push(char **args, pid_t newId);
void deleteNode(int position);


//Set2
int is_pipe(char *line);
int is_output(char *line);
int is_input(char *line);
int create_pipe(char *line);

//Set3
int initialize_shell();
void free_env_var();
void save_cmd(char *line);

#endif
