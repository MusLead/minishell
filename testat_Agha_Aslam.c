#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <signal.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

#define DEBUG 0 // if you want to disable the debug messages, just change this to 0

int last_status = 0;                   // Memory for return value
volatile sig_atomic_t child_count = 0; // Track number of child processes

/*
 * Displays the current path as shell prompt.
 * This loop splits the current working directory path by '/'.
 * The folder names are stored in an array, and only the last two entries
 * are printed to keep the shell prompt concise and informative.
 */
void print_prompt()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    { // > getcwd: get current location.
        char cwd_copy[PATH_MAX];
        strncpy(cwd_copy, cwd, PATH_MAX);

        char *folders[PATH_MAX];
        int count = 0;
        char *saveptr;
        char *token = strtok_r(cwd_copy, "/", &saveptr);

        while (token)
        {
            folders[count++] = token;
            token = strtok_r(NULL, "/", &saveptr);
        }

        if (count >= 2)
        {
            printf("%s/%s> ", folders[count - 2], folders[count - 1]);
        }
        else if (count == 1)
        {
            printf("%s> ", folders[0]);
        }
        else
        {
            printf("/> ");
        }
        fflush(stdout);
    }
    else
    {
        perror("getcwd error");
    }
}

// Check if process has children without blocking
int has_children()
{
    pid_t result = waitpid(-1, NULL, WNOHANG);

    if (result > 0)
    {
        // Child process was reaped, decrement counter
        if (child_count > 0)
            child_count--;
        return (child_count > 0);
    }
    else if (result == 0)
    {
        // Children exist but none have terminated
        return (child_count > 0);
    }
    else if (result == -1 && errno == ECHILD)
    {
        // No children exist
        child_count = 0;
        return 0;
    }
    else
    {
        // Error occurred, assume no children
        return 0;
    }
}

// Signal handling for Ctrl+C
void handle_sigint(int sig)
{
    // Only print the hint if we do not have any children
    if (!has_children())
    {
        // without this if-statement, the prompt will be printed twice, once here and once in the main loop
        printf("\n[Hint] Terminate the shell using the command 'exit'.\n");
        // > if there are still children, then just print the prompt again, so that the user can give another input.
        print_prompt();
        fflush(stdout);
    }
    else
    {
        // If there are children, just do not print the prompt and the Hint
        if (DEBUG)
            printf("[DEBUG] The programm is successfully terminated!\n");
        else
            printf("\n");
        fflush(stdout);
    }
}

// Display of last return value
void handle_sighup(int sig)
{
    if (sig == 0)
    {
        if (DEBUG)
            printf("[DEBUG] Last return value: %d\n", last_status);
        printf("%d\n", last_status);
    }
    else
        printf("\n[Hint] SIGHUP detected. Last return value: %d\n", last_status);
    fflush(stdout);
}

/*
 * Splits an input line into arguments for command execution.
 * The input_line is split into tokens by space character (' ').
 * Each token becomes an argument: the command itself, options, or filenames.
 * strtok_r modifies input_line by inserting null terminators and returns a pointer to each word.
 * The resulting args[] is a NULL-terminated array suitable for execvp().
 */
int tokenize_input(char *line, char *args[])
{

    // Iterate through the input_line and split it by spaces
    // strtok_r is used for thread-safe tokenization
    // we save each token (word of arguments) into the args array
    int argc = 0;
    char *saveptr; // state for strtok_r
    char *token = strtok_r(line, " ", &saveptr);

    while (token && argc < MAX_ARGS - 1) {
        args[argc++] = token;	// > token is the word within the command. it can be the command it self or the given parameter within the command
        token = strtok_r(NULL, " ", &saveptr);
    }

    args[argc] = NULL; // > to make sure that the arrays end! therefore NULL
    return argc;    
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
        char *args[MAX_ARGS]; // > copy the value of the commands into cmd_copy, so that it does not corrupt the data!
        tokenize_input(commands[i], args); // > take the command and convert them into string array, so that it could be executed with the given parameter through execv().

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

int main() {
    signal(SIGINT, handle_sigint); // Catch Ctrl+C
    signal(SIGHUP, handle_sighup); // Catch SIGHUP

    while (1) {
        print_prompt();
        char input_line[MAX_CMD_LEN];

        if (!fgets(input_line, MAX_CMD_LEN, stdin))
        {
            printf("\n");
            break; // EOF
            // diese Bereich ist problematisch -------------------------
            // if (feof(stdin))
            // {
            //     handle_sighup(1);
            //     clearerr(stdin);
            //     continue; // back to top of loop
            // }
            // else
            // {
            //     printf("[Hint] Unknown Inputs, exit terminal!\n");
            //     return 0;
            // }
            //----------------------------------------------------------
        }

        input_line[strcspn(input_line, "\n")] = '\0';
        if (DEBUG) printf("[DEBUG] shell_functionality, Input line: '%s'\n", input_line);

        char *saveptr;
        char *command = strtok_r(input_line, ";", &saveptr);

        while (command)
        {
            while (*command == ' ')
                command++;

            char *command_copy = strdup(command);
            if (!command_copy)
            {
                fprintf(stderr, "Error: Could not allocate memory for command.\n");
                break;
            }

            char *pipe = strchr(command, '|');
            if (pipe)
            {
                handle_multi_pipe(command);
                free(command_copy);
                command = strtok_r(NULL, ";", &saveptr);
                continue;
            }

            char *args[MAX_ARGS];
            tokenize_input(command_copy, args);

            if (!args || !args[0])
            {
                free(command_copy);
                
                command = strtok_r(NULL, ";", &saveptr);
                continue;
            }

            if (strcmp(args[0], "exit") == 0)
            {
                
                free(command_copy);
                printf("Shell terminated.\n");
                return EXIT_SUCCESS;
            }

            if (strcmp(args[0], "cd") == 0)
            {
                handle_cd(args);
                free(command_copy);
                
                command = strtok_r(NULL, ";", &saveptr);
                continue;
            }

            if (strcmp(args[0], "ret") == 0)
            {
                handle_sighup(0);
                
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
                if (DEBUG) printf("[DEBUG] Executing command: %s, with strchr: %s\n", args[0], strchr(args[0], '/'));
                if (strchr(args[0], '/') != NULL) {
                    execv(args[0], args);
                    perror("execv failed");
                } else {
                    execvp(args[0], args);
                    perror("execvp failed");
                }
                exit(1);
            }
            else
            {
                child_count++;
                int status;
                waitpid(pid, &status, 0);
                child_count--;
                last_status = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            }

            
            free(command_copy);
            command = strtok_r(NULL, ";", &saveptr);
        }

        if (DEBUG) printf("[DEBUG] Cleaning up all children processes...\n");
        while (has_children()) {
            wait(NULL);
        }
    }

    return 0;
}
