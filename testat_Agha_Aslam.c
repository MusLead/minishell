#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

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

int main() {
    char line[MAX_CMD_LEN];
    char *args[MAX_ARGS];

    while (1) {
        printf("→ ");
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

        // Tokenize using strtok_r
        tokenize_input(line, args);

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
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("fork");
            exit(1);
        }
    }

    return 0;
}
