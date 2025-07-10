/*
 * Author: Agha Muhammad Aslam
 * Description: This mini shell implements basic functions like command execution, directory changing,
 * pipelines (|), multiple commands in one input (;), displaying the last return value (ret), and capturing signals like Ctrl+C.
 * 
 * References:
 * execv vs execvp https://youtu.be/OVFEWSP7n8c?si=SyYfzKq_GNjYOmw-$0 
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <linux/limits.h>

#define MAX_ARGS 100 
#define MAX_LINE 1024

#define DEBUG 0 // if you want to disable the debug messages, just change this to 0

int last_status = 0;  // Memory for return value
volatile sig_atomic_t child_count = 0;  // Track number of child processes

/*
 * Displays the current path as shell prompt.
 * This loop splits the current working directory path by '/'.
 * The folder names are stored in an array, and only the last two entries
 * are printed to keep the shell prompt concise and informative.
 */
void print_prompt() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) { // > getcwd: get current location.
        char cwd_copy[PATH_MAX];
        strncpy(cwd_copy, cwd, PATH_MAX);

        char *folders[PATH_MAX];
        int count = 0;
        char *saveptr;
        char *token = strtok_r(cwd_copy, "/", &saveptr);

        while (token) {
            folders[count++] = token;
            token = strtok_r(NULL, "/", &saveptr);
        }

        if (count >= 2) {
            printf("%s/%s> ", folders[count - 2], folders[count - 1]);
        } else if (count == 1) {
            printf("%s> ", folders[0]);
        } else {
            printf("/> ");
        }
        fflush(stdout);
    } else {
        perror("getcwd error");
    }
}

// Check if process has children without blocking
int has_children() {
    pid_t result = waitpid(-1, NULL, WNOHANG);
    
    if (result > 0) {
        // Child process was reaped, decrement counter
        if (child_count > 0) child_count--;
        return (child_count > 0);
    } else if (result == 0) {
        // Children exist but none have terminated
        return (child_count > 0);
    } else if (result == -1 && errno == ECHILD) {
        // No children exist
        child_count = 0;
        return 0;
    } else {
        // Error occurred, assume no children
        return 0;
    }
}

// Signal handling for Ctrl+C
void handle_sigint(int sig) {
    // Only print the hint if we do not have any children
    if (!has_children()) { 
        // without this if-statement, the prompt will be printed twice, once here and once in the main loop
        printf("\n[Hint] Terminate the shell using the command 'exit'.\n");
        // > if there are still children, then just print the prompt again, so that the user can give another input.
        print_prompt(); 
        fflush(stdout);
    } else {
        // If there are children, just do not print the prompt and the Hint
        if(DEBUG) printf("[DEBUG] The programm is successfully terminated!\n");
        else printf("\n");
        fflush(stdout);
    }
}

// Display of last return value
void handle_sighup(int sig) {
    if(sig == 0) {
        if(DEBUG) printf("[DEBUG] Last return value: %d\n", last_status);
        printf("%d\n", last_status);
    }
    else printf("\n[Hint] SIGHUP detected. Last return value: %d\n", last_status);
    fflush(stdout);
}

/*
 * Splits an input line into arguments for command execution.
 * The input_line is split into tokens by space character (' ').
 * Each token becomes an argument: the command itself, options, or filenames.
 * strtok_r modifies input_line by inserting null terminators and returns a pointer to each word.
 * The resulting args[] is a NULL-terminated array suitable for execvp().
 */
char **parse_input(char *input_line) {
    // allocate memory for args array so that it can hold up to MAX_ARGS arguments
    char **args = malloc(MAX_ARGS * sizeof(char *));
    if (!args) return NULL;

    // Iterate through the input_line and split it by spaces
    // strtok_r is used for thread-safe tokenization
    // we save each token (word of arguments) into the args array
    int i = 0;
    char *saveptr;
    char *token = strtok_r(input_line, " ", &saveptr);

    while (token && i < MAX_ARGS - 1) {
        args[i++] = token;	// > token is the word within the command. it can be the command it self or the given parameter within the command
        token = strtok_r(NULL, " ", &saveptr);
    }

    args[i] = NULL; // > to make sure that the arrays end! therefore NULL
    return args;    // > args will be freed later after it is not being used anymore, especially in main!
}

/*
 * Handles the pipe between two or more programs
 * The pipe() system call creates a unidirectional communication channel.
 * For N piped commands, N-1 pipes are needed to transfer data from one command to the next.
 * Each pipe consists of a read and write end (pipefd[0] and pipefd[1]).
 * We use dup2() to redirect stdin and stdout to the appropriate pipe ends.
 */
void handle_multi_pipe(char *input) {
    // Split by '|'
    char *commands[MAX_ARGS];
    int count = 0;
    char *saveptr;

    char *token = strtok_r(input, "|", &saveptr);
    while (token && count < MAX_ARGS) {
        while (*token == ' ') token++; // skip leading spaces
        if (*token == '\0') {
            fprintf(stderr, "Error: Empty command between pipes not allowed.\n");
            return;
        }
        commands[count++] = token;
        token = strtok_r(NULL, "|", &saveptr);
    }

    if (count == 0) {
        fprintf(stderr, "Error: No valid command detected.\n");
        return;
    }

    // If original input starts or ends with '|', reject
    size_t len = strlen(input);
    if (input[0] == '|' || input[len - 1] == '|') {
        // > after I learn in the Uebung 6, I think it is better if I just set an error commands for the right one too.
        fprintf(stderr, "Error: Pipe at beginning or end not allowed.\n");
        return;
    }

    int **pipes = malloc((count - 1) * sizeof(int *));
    for (int i = 0; i < count - 1; ++i) {
        pipes[i] = malloc(2 * sizeof(int));
        if (pipe(pipes[i]) == -1) {
            perror("pipe failed");
            return;
        }
    }

    for (int i = 0; i < count; ++i) {
        char *cmd_copy = strdup(commands[i]); // > copy the value of the commands into cmd_copy, so that it does not corrupt the data!
        char **args = parse_input(cmd_copy); // > take the command and convert them into string array, so that it could be executed with the given parameter through execv().

        pid_t pid = fork();
        if (pid == 0) {
            // > how the dup works https://youtu.be/PIb2aShU_H4?si=WET26X4zSAwlRPhu$0
            if (i > 0) { // Not first: read from previous pipe
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < count - 1) { // Not last: write to next pipe
                dup2(pipes[i][1], STDOUT_FILENO); // > take the Process content of the index pipefd[1] and into the index of STDOUT located, so that the stdout (output) values will be redirected into the pipe.
            }

            // Close all pipes in child, because dup2()-functionality: duplicates the file descriptors (see Ueubng 6)
            for (int j = 0; j < count - 1; ++j) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(args[0], args); // > execvp means execute program with vector (array) as input and uses path to look for the program
            perror("execv failed");
            exit(1);
        } else if (pid > 0) {
            // Parent process: increment child counter
            child_count++;
        }

        free(cmd_copy);
        free(args);
    }

    // Close all pipes in parent
    for (int i = 0; i < count - 1; ++i) {
        close(pipes[i][0]);
        close(pipes[i][1]);
        free(pipes[i]);
    }
    free(pipes);

    // Wait for all children
    int status;
    for (int i = 0; i < count; ++i) {
        wait(&status);
        if (child_count > 0) child_count--;
    }

    if (WIFEXITED(status)) last_status = WEXITSTATUS(status);
    else last_status = -1;
}

// Handles the 'cd' command to change directories
void handle_cd(char **args)
{
    const char *target = args[1] ? args[1] : getenv("HOME");
    char path[PATH_MAX];

    if (args[1] && args[1][0] == '~')
    {
        const char *home = getenv("HOME");
        snprintf(path, sizeof(path), "%s%s", home, args[1] + 1); // > if the first character of args[1] is '~', then we take the home directory and concatenate it with the rest of the path.
        target = path;
    }

    if (chdir(target) != 0) // > chdir: change directory to the given path
    {
        fprintf(stderr, "cd failed: %s\n", strerror(errno)); // > send a message why it does not work.
        last_status = 1;
    }
    else
    {
        last_status = 0;
    }
}

int shell_functionality(int *retFlag) {
    *retFlag = 1;
    print_prompt();
    char input_line[MAX_LINE];

    if (!fgets(input_line, MAX_LINE, stdin))
    {
        if (feof(stdin))
        { // > ctr + d sends EOF (End-Of-File) signal. Therefore we should use this feof() instead
            handle_sighup(1);
            clearerr(stdin); // > Needed, because otherwise the Comand Line will behave like the input is always feof.
            {
                *retFlag = 3;
                return 0;   // > No need to read the input when this signal is being given, just go to the next iteration and wait until someone give an inputs!
            };              
        }
        else
        {
            printf("[Hint] Unknown Inputs, exit terminal!");
            return EXIT_FAILURE;
        }
    }

    input_line[strcspn(input_line, "\n")] = '\0'; // > change at the end of the line of the code
    if(DEBUG) printf("[DEBUG] shell_functionality, Input line: '%s'\n", input_line); // > for debugging purpose, so that I can see what is being given as input
    char *saveptr;
    char *command = strtok_r(input_line, ";", &saveptr); // > collect the collection of Strings of command, especially if there is ";"

    // > The `;` is being used as the iterator through the commands that might be within the command#s array
    // execute each command in the input line e.g. `ls; pwd; echo "Hello World"; cd /tmp`
    // it executes ls first, then pwd, then echo "Hello World", and finally cd /tmp
    while (command)
    {
        while (*command == ' ')
            command++; // > if within the char array there are blank space, then skip it and go to the next character!

        char *command_copy = strdup(command); // > copy the command into command_copy in the heap_memorry, so that it does not affect the pointer variable command when we are passing other value to other function.
        if (!command_copy)
        {
            fprintf(stderr, "Error: Could not allocate memory for command.\n");
            break;
        }

        // Detect pipe command
        char *pipe = strchr(command, '|');
        if (pipe)
        {
            handle_multi_pipe(command);
            command = strtok_r(NULL, ";", &saveptr); // > go to to the next available command, take the pointer to the next command.
            continue;                                // > For the next command, we go back and see if there is Pipe or simmilar.
        }

        char **args = parse_input(command_copy); // > take the command and convert them into string array, so that it could be executed with the given parameter through execv().
        if (!args || !args[0])
        { // > if there is no command or pointers being return from parse_input, then go to the next iteration and see if there is
          // > another command that can be executed. If not then the `while(command)` ended and wait for the next input.
            free(command_copy);
            free(args);
            command = strtok_r(NULL, ";", &saveptr);
            continue;
        }

        // Built-in: exit
        if (strcmp(args[0], "exit") == 0)
        {
            free(args);
            free(command_copy);
            printf("Shell terminated.\n");
            return EXIT_SUCCESS;
        }

        // Built-in: cd
        if (strcmp(args[0], "cd") == 0)
        {
            handle_cd(args);
            free(command_copy);
            free(args);                              // > This is important only if the args is being allocated in the heap memory, otherwise it will cause a memory leak!
                                                     // > There was problem in the test case, where the args was not being freed and crahsed, therefore it caused a memory leak.
            command = strtok_r(NULL, ";", &saveptr); // > if there is no command or pointers being return from parse_input, then go to the next iteration and see if there is
                                                     // > another command that can be executed. If not then the `while(command)` ended and wait for the next input.
            continue;
        }

        // Built-in: ret
        if (strcmp(args[0], "ret") == 0)
        {
            handle_sighup(0);
            free(args);
            free(command_copy);
            command = strtok_r(NULL, ";", &saveptr);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork failed");
            last_status = 1;
        }
        else if (pid == 0)
        {
            // Child Process = execute the command
            if(DEBUG) printf("[DEBUG] Executing command: %s, with strchr: %s\n", args[0], strchr(args[0], '/'));
            if (strchr(args[0], '/') != NULL) {
                // If the command contains a '/', treat it as a path
                execv(args[0], args);  // exact path
                perror("execv failed");
            } else {
                execvp(args[0], args); // search PATH
                perror("execvp failed");
            }
            exit(1);
        }
        else
        {
            // Parent Process = wait for the command to be ended
            child_count++; // Increment child counter
            int status;
            waitpid(pid, &status, 0);
            child_count--; // Decrement child counter when child finishes
            // > see this reference for more information about WIFEXITED https://www.ibm.com/docs/xl-fortran-aix/16.1.0?topic=procedures-wifexitedstat-val
            // > Reason: WIFEXITED if the process is not exited, then it will return 0, otherwise it will return non zero value
            last_status = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        }

        free(args);
        free(command_copy);
        command = strtok_r(NULL, ";", &saveptr);
    }
    *retFlag = 0;
    return 0; // > return 0 means that the command is being executed successfully and there is no error
}

int main() {
    signal(SIGINT, handle_sigint); // Catch Ctrl+C
    signal(SIGHUP, handle_sighup); // Catch SIGHUP

    while (1) {
        int retFlag;
        int retVal = shell_functionality(&retFlag);
        if (retFlag == 3)
            continue;
        if (retFlag == 1)
        if (retFlag == 1) {
            // clean up ALL children, so that the processes are not left hanging
            if (DEBUG) printf("[DEBUG] Cleaning up all children processes...\n");
            while (has_children()) {
                wait(NULL);
            }
            return retVal;
        }
    }

    return 0;
}
