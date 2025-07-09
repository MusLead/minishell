#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

int main() {
    char line[MAX_CMD_LEN];

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

        // Tokenize input
        char *args[MAX_ARGS];
        int argc = 0;

        char *token = strtok(line, " ");
        while (token != NULL && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;

        

        pid_t pid = fork();

        if (pid == 0) {
            // Child process

            if (strcmp(args[0], "/bin/wc") == 0) {
                // For demonstration: using execv, since it's fully qualified
                execv(args[0], args);
                perror("execv");
                exit(1);
            } else {
                execvp(args[0], args);
                perror("execvp");
                exit(1);
            }

        } else if (pid > 0) {
            // Parent process waits
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("fork");
            exit(1);
        }
    }

    return 0;
}
