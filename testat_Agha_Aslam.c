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


int last_status = 0;  // Memory for return value
volatile sig_atomic_t child_count = 0;  // Track number of child processes

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

// Tokenize input safely using strtok_r
int tokenize_input(char *line, char *args[]) {
    int argc = 0;
    char *saveptr;  // state for strtok_r
    char *token = strtok_r(line, " ", &saveptr);

    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok_r(NULL, " ", &saveptr);
    }
    args[argc] = NULL;
    return argc;
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

    char line[MAX_CMD_LEN];
    char *args[MAX_ARGS];

    while (1) {
        print_prompt();
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break; // EOF
        }

        // Remove newline
        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) continue;

        // Built-in exit
        if (strcmp(line, "exit") == 0) {
            printf("Bye!\n");
            break;
        }

        // Built-in: ret
        if (strcmp(line, "ret") == 0)
        {
            handle_sighup(0);
            //free(line);
            continue;
        }

        // Tokenize using strtok_r
        tokenize_input(line, args);

        // built-in: cd
        if (strncmp(line, "cd", 2) == 0)
        {
            handle_cd(args);
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            // Child process

            if (strcmp(args[0], "/bin/wc") == 0) {
                execv(args[0], args);
                perror("execv");
                exit(1);
            } else {
                execvp(args[0], args);
                perror("execvp");
                exit(1);
            }

        } else if (pid > 0) {
            child_count++; // Increment child counter
            int status;
            waitpid(pid, &status, 0);
            child_count--; // Decrement child counter when child finishes
            last_status = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        } else {
            perror("fork");
            last_status = 1;
            exit(1);
        }
    }

    return 0;
}
