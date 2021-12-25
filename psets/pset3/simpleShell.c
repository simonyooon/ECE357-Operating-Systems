/*
ECE 357: Computer Operating Systems  
Simon Yoon
*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <fcntl.h>

enum redirection {redirect_Open = 0, redirect_CreateTruncate, redirect_CreateAppend };

int mysh(FILE *input);
int tokenize(char *line, char **tokens);
int parse(char ** tokens, FILE *input);
int IOredirect(char **tokens);
int redirecthelper(char **tokens, int size, int redirected, enum redirection val);


int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "Correct Usage: /shell [script-file]\n");
        return -1;
    }
    else if (argc == 2) {
        FILE *input;
        if (!(input = fopen(argv[1], "r"))) {
            fprintf(stderr, "Unable to open script file %s: %s\n", argv[1], strerror(errno));
            return -1;
        }

        return mysh(input);
    }
    else 
        return mysh(stdin);
}


// shell
int mysh(FILE *input) {
    char *buffer;
    if (!(buffer = malloc(sizeof(char)*4096))) { // max length of stdin
        fprintf(stderr, "Unable to allocate buffer in order to read from input: %s\n", strerror(errno));
        return -1;
    }

    int getline_ret;
    size_t n = 4096; // max length of stdin
    int exit_status = 0;
    while ( (getline_ret = getline(&buffer, &n, input)) != -1) {
        
        char **tokens; // parse curLine
        if (!(tokens = malloc(sizeof(char *)*2048))) { // max args in stdin
            fprintf(stderr, "Error allocating space for stdin line: %s\n", strerror(errno));
            return -1;
        }

        if (tokenize(buffer, tokens) == -1) {
            return -1;
        }

        // check if commands need, else exit
        if (!strcmp(tokens[0], "exit")) {
            if (!tokens[1])
                exit(exit_status);
            else
                exit(atoi(tokens[1]));
        }
        else
            exit_status = parse(tokens, input);

        free(tokens);
    }

    if (errno) {
        fprintf(stderr, "Error Reading in from stdin: %s\n", strerror(errno));
        return -1;
    }
    
    free(buffer);
    return exit_status;
}


// takes strings and parses, returns tokens
// 0 on success, -1 if too many args
int tokenize(char *line, char **tokens) {
    while (line[0] == ' ') // skip spaces 
        ++line;
    
    int index = 0;

    const char delim[2] = " ";
    char *token = strtok(line, delim);
    while (token && index < 2048) { // max args in stdin
        if (token[strlen(token)-1] == '\n') // pop newline 
            token[strlen(token)-1] = 0;
        if (strcmp(token, "\n")) // if not newline add to token
            tokens[index] = token;
        
        ++index;
        token = strtok(NULL, delim);
    }

    if (index == 2048) { // max args in stdin
        fprintf(stderr, "Too many arguments in stdin line.\n");
        return -1;
    }
    tokens[index] = NULL;
    return 0;
}

// takes a tokenized line and parses
// ignores after #, change directories with cd {dir}, 
// prints working directory pwd, exit
// returns last spawned child status
int parse(char **tokens, FILE *input) {
    // built in command check
    if (tokens[0][0] == '#') // ignore after #
        return 0;
    else if (!strcmp(tokens[0], "cd")) { 
        if (!tokens[1] || chdir(tokens[1])) {
            fprintf(stderr, "Error changing directories: %s\n", strerror(errno));
            return -1;
        }
        return 0;
    }
    else if (!strcmp(tokens[0], "pwd")) {
        char *path;
        if (!(path = malloc(sizeof(char)*1024))) { // max length of file path
            fprintf(stderr, "Error allocating memory for current path name: %s\n", strerror(errno));
            return -1;
        }

        if (!getcwd(path, 1024)) { //max length of file path
            fprintf(stderr, "Error finding current path name: %s\n", strerror(errno));
            return -1;
        }

        fprintf(stderr, "%s\n", path); // no I/O redirect for commands
        free(path);
        return 0;
    }

    // fork, exec, time
    int pid;
    int status;
    
    clock_t start, end;
    struct tms start_time, end_time;
    
    if ((start = times(&start_time)) == -1) {
        fprintf(stderr, "Error getting start time of command:%s\n", strerror(errno));
        return -1;
    }

    if ((pid = fork()) == -1) {
        fprintf(stderr, "Error during process forking: %s\n", strerror(errno));
        return -1;
    }
    else if (pid == 0) { // child
        int command_size;
        if ((command_size = IOredirect(tokens)) == -1)
            exit(-1);
        
        tokens[command_size] = NULL;

        if (input != stdin) { // close script
            if (!fclose(input)) {
                fprintf(stderr, "Error closing input file before exec: %s\n", strerror(errno));
                return -1;
            }
        }   
        if (execvp(tokens[0], tokens) == -1)
            exit(-1);
    }
    else { // parent
        if (wait(&status) == -1 || !WIFEXITED(status)) { // wait for child
            fprintf(stderr, "Child process %d exited in error: %s\n", pid, strerror(errno));
            return -1;
        }

        if ((end = times(&end_time)) == -1) {
            fprintf(stderr, "Error getting end time of command:%s\n", strerror(errno));
            return -1;
        }

        long clocktick;
        if ((clocktick = sysconf(_SC_CLK_TCK)) == -1) {
            fprintf(stderr, "Error acquiring clock ticks per second from sysconf:%s\n", strerror(errno));
            return -1;
        };

        // child stat
        fprintf(stderr, "Child process %d exited normally,\n", pid);
        fprintf(stderr, "Real: %f, User: %f, System: %f.\n",
                    (end - start) / (double) clocktick, 
                    (end_time.tms_cutime - start_time.tms_cutime) / (double) clocktick,
                    (end_time.tms_cstime - start_time.tms_cstime) / (double) clocktick);   
    }
    return status;
}


// takes ptr to strings, looks for i/o redirect and executes
// returns # of commands/args before redirect
int IOredirect(char **tokens) {

    int init_io_index = -1;
    int size = 0;
    for (char *cur = tokens[size] ; cur != NULL ; cur = tokens[++size]) {
        if (strncmp(cur, "2>", 2) == 0) { // stderr redirect
            if (init_io_index  == -1)
                init_io_index = size;

            if (fileno(stderr) != 2) {  // check for multiple fd redirect
                fprintf(stderr, "Cannot redirect a file more than once in a single command launch.\n");
                return -1; 
            }

            if (cur[2] == '>' && redirecthelper(tokens, size, STDERR_FILENO, redirect_CreateAppend) == -1)
                    return -1;
            else if (redirecthelper(tokens, size, STDERR_FILENO, redirect_CreateTruncate) == -1)
                    return -1;
        }

        else if (cur[0] == '>') { // stdout redirect
            if (init_io_index  == -1)
                init_io_index = size;
            
            if (fileno(stdout) != 1) { // check for multiple fd redirect
                fprintf(stderr, "Cannot redirect a file more than once in a single command launch.\n");
                return -1; 
            }

            if ( (cur[1] == '>' && redirecthelper(tokens, size, STDOUT_FILENO, redirect_CreateAppend) == -1))
                return -1;
            else if (redirecthelper(tokens, size, STDOUT_FILENO, redirect_CreateTruncate) == -1)
                return -1;
        }
        else if (cur[0] == '<') { // stdin redirect
            if (init_io_index  == -1)
                init_io_index = size;

            if (fileno(stdin) != 0) { // check for multiple fd redirect
                fprintf(stderr, "Cannot redirect a file more than once in a single command launch.\n");
                return -1;
            }

            if (redirecthelper(tokens, size, STDIN_FILENO, redirect_Open) == -1)
                return -1;
        }
    }
    return (init_io_index == -1) ? size : init_io_index;
}


// helper for i/o redirect
// takes file name for redirection, executes, error checking 
int redirecthelper(char **tokens, int size, int redirected, enum redirection val) {
    char *file_name = tokens[size+1];  // filename
    char *cur = tokens[size];
    // get name of subject to be redirected to
    if (val == redirect_CreateAppend && redirected == STDERR_FILENO) {
        if (strlen(cur) > 3)
            file_name = tokens[size]+3;
    }
    else if (val == redirect_CreateAppend || (val == redirect_CreateTruncate && redirected == STDERR_FILENO)) {
        if (strlen(cur) > 2)
            file_name = tokens[size]+2;
    }
    else if (strlen(cur) > 1)
            file_name = tokens[size]+1;

    if (!file_name || strstr(file_name, "<") || strstr(file_name, ">")) {
        fprintf(stderr, "Invalid I/O redirection command %s.\n", cur);
        return -1;
    }

    // open file to redirect
    int new_file;
    if (val == redirect_Open) {
        if ((new_file = open(file_name, O_RDONLY, 0666)) == -1) {
            fprintf(stderr, "Error opening %s for I/O redirection: %s\n", file_name, strerror(errno));
            return -1;
        }
    }
    else if (val == redirect_CreateAppend) {
        if ((new_file = open(file_name, O_WRONLY|O_CREAT|O_APPEND, 0666)) == -1) {
            fprintf(stderr, "Error opening %s for I/O redirection: %s\n", file_name, strerror(errno));
            return -1;
        }
    }
    else {
        if ((new_file = open(file_name, O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1) {
            fprintf(stderr, "Error opening %s for I/O redirection: %s\n", file_name, strerror(errno));
            return -1;
        }
    }

    if (dup2(new_file, redirected) < 0) { // update redirected
        fprintf(stderr, "Error redirecting I/O: %s\n", strerror(errno));
        return -1;
    }
    if (close(new_file) == -1) {
        fprintf(stderr, "Error closing extra reference to I/0 during redirection: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}