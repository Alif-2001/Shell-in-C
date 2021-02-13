#    Unix Shell in C

##   Run the shell
1. Run "make"
2. Then run the command "./myShell" to start the shell

The shell supports the following set of functions.

## Set 1:

-> Supports commands with and without arguments
    example: ls or ls -l

-> Runs commands that end with '&' in the background
    example: sleep 10 &

## Set 2:

-> Supports input and output redirection
    example: ls > output.txt or wc < input.txt

-> Supports multiple levels of pipes
    example: ls | sort | wc

## Set 3:

-> Has the following environment variables:
    PATH, HISTFILE, HOME.
    Use 'echo' to print their values.
    example: echo $PATH

-> Imports PATH, HOME, and HISTFILE variables from
    .CIS3110_profile. if the file is not present in
    the current working directory, a new file is created
    with default environment variables.

-> Has other built-in functions:
    - export:   use it to set the environment variables
                example: export PATH=usr/bin:$HOME
    
    - history:  prints out all previous history to the shell
                options: -c = clears the history, n = to print 
                n last commands
    
    - cd:       to change directory


Everything seems to be working!
