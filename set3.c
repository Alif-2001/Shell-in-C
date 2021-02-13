#include "shell.h"

/*code copied from https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/ 
    The function replaces a word (oldW) in a string (s) with a new one (newW).
    I couldn't find a library that does that in C. I use it in a couple of places 
    throughout set3, mainly when creating path names to open files */
char* replaceWord(const char* s, const char* oldW, const char* newW);

/* checks the paths and replaces any refrences with their actual path
    ex: if $PATH is found in HOME path, it will be replace with
    it's original path '/bin' */
void check_and_replace();

/* Updates the environment varibales */
void update_environment();

/* Opens and Updates .CIS3110_profile file
    by putting the current path variables */
void update_profile_file();

/* creates a filename based on the environement variables */
char* create_file_path(struct envVariable var, char *toReplace, char *filename);

/* creates and initializes the environment variables */
void initialize_env_vars(){
    Path.path = malloc(BUFSIZE);
    Home.path = malloc(BUFSIZE);
    Hist.path = malloc(BUFSIZE);
    strcpy(Path.path, "PATH=/usr/bin:/bin");
    char *cwd = malloc(BUFSIZE);
    strcpy(Home.path, "HOME=");
    strcat(Home.path,getcwd(cwd, BUFSIZE));
    strcpy(Hist.path, "HISTFILE=");
    strcat(Hist.path,getcwd(cwd, BUFSIZE));
    free(cwd);
}

/* free the environment variables */
void free_env_var() {
    free(Path.path);
    free(Home.path);
    free(Hist.path);
}

/* change the current directory using chdir() */
int sh_cd(char **args) {
    if(args[1]){
        if(strcmp(args[1],"~") == 0){
            chdir(getenv("HOME"));
        }else if(chdir(args[1]) != 0){
            printf("%s: No such directory\n", args[1]);
        }
    }
    return 1;
}

/* change environment variables using 'export PATH=..' */
int sh_export(char **args) {
    if(strstr(args[1],"PATH=") != NULL){
        strcpy(Path.path, args[1]);
    }else if(strstr(args[1],"HOME=") != NULL){
        strcpy(Home.path, args[1]);
    }else if(strstr(args[1],"HISTFILE=") != NULL){
        strcpy(Hist.path, args[1]);
    }else{
        printf("Invalid variable");
    }
    update_environment();
    update_profile_file();
    return 1;
}

/* print the user input, but variable names
    ex: 'echo $PATH' would print out the location
    stored in the path variable */
int sh_echo(char **args) {
    if(strcmp(args[1],"$HOME") == 0){
        printf("%s\n", getenv("HOME"));
    }else if(strcmp(args[1],"$PATH") == 0){
        printf("%s\n", getenv("PATH"));
    }else if(strcmp(args[1],"$HISTFILE") == 0){
        printf("%s\n", getenv("HISTFILE"));
    }else{
        int i = 1;
        while(args[i] != NULL){
            printf("%s\n",args[i]);
            i++;
        }
    }
    return 1;
}

/* checks if the string provided is a didgit */
int is_digit(char *str){
    int i = 0;
    while(str[i] != '\0'){
        if(!isdigit(str[i])){       //go through each char in the string
            return 0;               //and check if it's a digit.
        }
        i++;
    }
    return 1;
}

/* Opens the history file and prints n number of history */
void print_n_history(char *filename, int n){
    
    FILE *fp = fopen(filename,"r");
    int count = 0;
    char *line = malloc(BUFSIZE);
    while(fgets(line,BUFSIZE,fp)){
        count++;
    }
    
    fp = fopen(filename,"r");

    int c = count - n;
    if (c <= 0) {
        c = 0;      //if count exceeds the number of args in history, then print everything that is in the file
    }
    count = 0;
    while(fgets(line,BUFSIZE,fp)){
        if(count >= c){
            printf(" %d  %s",count, line);
        }
        count++;
    }

    free(line);
    fclose(fp);
}

/* print the history by opening the .CIS3110_history file */
int sh_history(char **args) {
    char *filename = create_file_path(Hist, "HISTFILE=", "/.CIS3110_history");
    if (args[1] != NULL) {
        if (strcmp(args[1],"-c") == 0) {    //check if there -c, and clear the file
            FILE *fp = fopen(filename,"w");
            printf("History cleared.\n");
            fclose(fp);
        }else if(is_digit(args[1])){    //check if there s a digit
            int n = atoi(args[1]);
            print_n_history(filename, n);   //if yes, print n last commands

        }
    }else{
        FILE *fp = fopen(filename,"r");
        int count = 1;
        char *line = malloc(BUFSIZE);
        while(fgets(line,BUFSIZE,fp)){
            printf(" %d  %s", count, line);
            count++;
        }
        free(line);
        fclose(fp);
    }
    free(filename);
    return 1;
}

/* Opens and Updates .CIS3110_profile file
    by putting the current path variables */
void update_profile_file() {
    char *filename = create_file_path(Home, "HOME=", "/.CIS3110_profile");
    FILE *fp = fopen(filename,"w");
    check_and_replace();
    fprintf(fp, "export %s\n",Path.path);
    fprintf(fp, "export %s\n", Home.path);
    fprintf(fp, "export %s\n", Hist.path);
    fclose(fp);
    free(filename);
}

/* This file opens the .CIS3110_profile and updates the environment vars.
    If it's not present then it creates the file and adds the default
    paths. */
int read_setup_files() {
    char *filename = create_file_path(Home, "HOME=", "/.CIS3110_profile");
    FILE*fp = fopen(filename,"r");

    if(fp==NULL){
        printf("No profile file.\nSetting Path to default.\n");
        update_profile_file();
    }else{
        char * cmd = malloc(BUFSIZE);
        char * path = malloc(BUFSIZE);
        while (fscanf(fp,"%s %[^\n]", cmd, path) != EOF){
            if(strstr(path,"HOME=") != NULL){
                strcpy(Home.path, path);
            }
            if(strstr(path,"PATH=") != NULL){
                strcpy(Path.path, path);
            }
            if(strstr(path,"HISTFILE=") != NULL){
                strcpy(Hist.path, path);
            }
        }
        fclose(fp);
        free(cmd);
        free(path);
    }
    update_environment();
    free(filename);
    return 1;
}

/* initializes environment variables and reads the set up file (.CIS3110_profile) */
int initialize_shell() {
    initialize_env_vars();
    read_setup_files();
    return 1;
}

/* Updates the environment varibales */
void update_environment() {
    check_and_replace();
    putenv(Path.path);
    putenv(Home.path);
    putenv(Hist.path);
}

/* Saves the comand the .CIS3110_history file */
void save_cmd(char *line) {
    char *filename = create_file_path(Hist, "HISTFILE=", "/.CIS3110_history");
    FILE *fp = fopen(filename,"a");
    fprintf(fp, "%s", line);
    fclose(fp);
    free(filename);
}

/* Creates a filename based on the environement variables */
char* create_file_path(struct envVariable var, char *toReplace, char *filename){
    char * path = malloc(BUFSIZE);
    char * replace = replaceWord(var.path, toReplace,"");
    strcpy(path,replace);
    strcat(path,filename);
    free(replace);
    return path;
}

/*code copied from https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/ 
    The function replaces a word (oldW) in a string (s) with a new one (newW).
    I couldn't find a library that does that in C. I use it in a couple of places 
    throughout set3, mainly when creating path names to open files */

char* replaceWord(const char* s, const char* oldW, 
                  const char* newW) 
{  
    char* result; 
    int i, cnt = 0; 
    int newWlen = strlen(newW); 
    int oldWlen = strlen(oldW);
    
    // Counting the number of times old word 
    // occur in the string 
    for (i = 0; s[i] != '\0'; i++) { 
        if (strstr(&s[i], oldW) == &s[i]) { 
            cnt++; 
  
            // Jumping to index after the old word. 
            i += oldWlen - 1; 
        } 
    }

    // Making new string of enough length 
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1); 
    i = 0; 

    while (*s) { 
        // compare the substring with the result 
        if (strstr(s, oldW) == s) { 
            strcpy(&result[i], newW); 
            i += newWlen; 
            s += oldWlen; 
        } 
        else
            result[i++] = *s++; 
    } 
    result[i] = '\0'; 
    return result; 
}

/* checks the paths and replaces any refrences with their actual path
    ex: if $PATH is found in HOME path, it will be replace with
    it's original path '/bin' */
void check_and_replace(){
    char toReplace[3][10] = {
        "$HOME",
        "$PATH",
        "$HISTFILE"
    };
    char * repl1 = NULL;
    char * repl2 = NULL;
    for(int i = 0; i < 3; i++) {
        if(strstr(Path.path,toReplace[i]) != NULL){
            if(i==0){
                repl1 = replaceWord(Home.path,"HOME=","");
                repl2 = replaceWord(Path.path,toReplace[i],repl1);
                strcpy(Path.path,repl2);
            }
            if(i==1){
                repl1 = replaceWord(Path.path,"PATH=","");
                repl2 = replaceWord(Path.path,toReplace[i],repl1);
                strcpy(Path.path,repl2);
            }
            if(i==2){
                repl1 = replaceWord(Hist.path,"HISTFILE=","");
                repl2 = replaceWord(Path.path,toReplace[i],repl1);
                strcpy(Path.path,repl2);
            }
        }
        
        if(strstr(Hist.path,toReplace[i]) != NULL){
            if(i==0){
                repl1 = replaceWord(Home.path,"HOME=","");
                repl2 = replaceWord(Hist.path,toReplace[i],repl1);
                strcpy(Hist.path,repl2);
            }
            if(i==1){
                repl1 = replaceWord(Path.path,"PATH=","");
                repl2 = replaceWord(Hist.path,toReplace[i],repl1);
                strcpy(Hist.path,repl2);
            }
            if(i==2){
                repl1 = replaceWord(Hist.path,"HISTFILE=","");
                repl2 = replaceWord(Hist.path,toReplace[i],repl1);
                strcpy(Hist.path,repl2);
            }
        }

        if(strstr(Home.path,toReplace[i]) != NULL){
            if(i==0){
                repl1 = replaceWord(Home.path,"HOME=","");
                repl2 = replaceWord(Home.path,toReplace[i],repl1);
                strcpy(Home.path,repl2);
            }
            if(i==1){
                repl1 = replaceWord(Path.path,"PATH=","");
                repl2 = replaceWord(Home.path,toReplace[i],repl1);
                strcpy(Home.path,repl2);
            }
            if(i==2){
                repl1 = replaceWord(Hist.path,"HISTFILE=","");
                repl2 = replaceWord(Home.path,toReplace[i],repl1);
                strcpy(Home.path,repl2);
            }
        }
    }
    free(repl1);
    free(repl2);
}
