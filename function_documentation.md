# Function Documentation for testat_*.c

**Author:** Agha Muhammad Aslam  
**Description:** Documentation of all functions used in the mini-shell implementation

---

## Table of Contents

1. [Standard C Library Functions](#standard-c-library-functions)
   - [stdio.h Functions](#stdioh-functions)
   - [stdlib.h Functions](#stdlibh-functions)
   - [unistd.h Functions](#unistdh-functions)
   - [string.h Functions](#stringh-functions)
   - [sys/wait.h Functions](#syswaith-functions)
   - [signal.h Functions](#signalh-functions)
2. [Custom Functions](#custom-functions)
3. [Constants and Macros](#constants-and-macros)
4. [Global Variables](#global-variables)

---

## Standard C Library Functions

### stdio.h Functions

#### `printf()`

- **Definition:** `int printf(const char *format, ...)`
- **Usage in Code:** Displaying prompts, debug messages, and status information
- **Examples in Code:**
  - **print_prompt():** `printf("%s/%s> ", folders[count - 2], folders[count - 1]);` - Shell prompt display
  - **print_prompt():** `printf("%s> ", folders[0]);` - Single folder prompt
  - **print_prompt():** `printf("/>  ");` - Root directory prompt
  - **handle_sigint():** `printf("\n[Hinweis] Beenden Sie die Shell mit dem Befehl 'exit'.\n");` - SIGINT message
  - **handle_sigint():** `printf("[DEBUG] The programm is successfully terminated!\n");` - Debug message
  - **handle_sighup():** `printf("Letzter Rueckgabewert: %d\n", last_status);` - Status display
  - **handle_sighup():** `printf("\n[Hinweis] SIGHUP erkannt. Letzter Rueckgabewert: %d\n", last_status);` - SIGHUP message
  - **shell_functionality():** `printf("[Hinweis] Unknown Inputs, exit terminal!");` - Error message
  - **shell_functionality():** `printf("[DEBUG] Eingabezeile: '%s'\n", input_line);` - Debug input display
  - **shell_functionality():** `printf("Shell beendet.\n");` - Exit message

#### `fprintf()`

- **Definition:** `int fprintf(FILE *stream, const char *format, ...)`
- **Usage in Code:** Printing error messages to stderr
- **Examples in Code:**
  - **handle_multi_pipe():** `fprintf(stderr, "Fehler: Leeres Kommando zwischen Pipes nicht erlaubt.\n");`
  - **handle_multi_pipe():** `fprintf(stderr, "Fehler: Kein gültiges Kommando erkannt.\n");`
  - **handle_multi_pipe():** `fprintf(stderr, "Fehler: Pipe am Anfang oder Ende nicht erlaubt.\n");`
  - **handle_cd():** `fprintf(stderr, "cd fehlgeschlagen: %s\n", strerror(errno));`
  - **shell_functionality():** `fprintf(stderr, "Fehler: Speicher für das Kommando konnte nicht reserviert werden.\n");`

#### `fgets()`

- **Definition:** `char *fgets(char *s, int size, FILE *stream)`
- **Usage in Code:** Reading user input from stdin
- **Example in Code:**
  - **shell_functionality():** `if (!fgets(input_line, MAX_LINE, stdin))` - Main input reading
- **Note:** Returns NULL on error or EOF

#### `fflush()`

- **Definition:** `int fflush(FILE *stream)`
- **Usage in Code:** Ensuring output is immediately displayed
- **Examples in Code:**
  - **print_prompt():** `fflush(stdout);` - Flush prompt output
  - **handle_sigint():** `fflush(stdout);` - Flush SIGINT message
  - **handle_sigint():** `fflush(stdout);` - Flush debug message
  - **handle_sighup():** `fflush(stdout);` - Flush status message

#### `perror()`

- **Definition:** `void perror(const char *s)`
- **Usage in Code:** Displaying system error messages
- **Examples in Code:**
  - **print_prompt():** `perror("getcwd error");` - Directory error
  - **handle_multi_pipe():** `perror("pipe fehlgeschlagen");` - Pipe creation error
  - **handle_multi_pipe():** `perror("execv fehlgeschlagen");` - Exec error in pipe
  - **shell_functionality():** `perror("fork fehlgeschlagen");` - Fork error
  - **shell_functionality():** `perror("execvp fehlgeschlagen");` - Exec error in main command

#### `feof()`

- **Definition:** `int feof(FILE *stream)`
- **Usage in Code:** Checking if EOF (Ctrl+D) was reached
- **Example in Code:**
  - **shell_functionality():** `if (feof(stdin))` - Detects Ctrl+D input

#### `clearerr()`

- **Definition:** `void clearerr(FILE *stream)`
- **Usage in Code:** Resetting stdin state after EOF
- **Example in Code:**
  - **shell_functionality():** `clearerr(stdin);` - Prevents continuous EOF state

#### `snprintf()`

- **Definition:** `int snprintf(char *str, size_t size, const char *format, ...)`
- **Usage in Code:** Building file paths for tilde (~) expansion
- **Example in Code:**
  - **handle_cd():** `snprintf(path, sizeof(path), "%s%s", home, args[1] + 1);` - Tilde expansion

---

### stdlib.h Functions

#### `malloc()`

- **Definition:** `void *malloc(size_t size)`
- **Usage in Code:** Allocating memory for argument arrays and pipe arrays
- **Examples in Code:**
  - **parse_input():** `char **args = malloc(MAX_ARGS * sizeof(char *));` - Argument array allocation
  - **handle_multi_pipe():** `int **pipes = malloc((count - 1) * sizeof(int *));` - Pipe array allocation
  - **handle_multi_pipe():** `pipes[i] = malloc(2 * sizeof(int));` - Individual pipe allocation

#### `free()`

- **Definition:** `void free(void *ptr)`
- **Usage in Code:** Deallocating dynamically allocated memory
- **Examples in Code:**
  - **handle_multi_pipe():** `free(cmd_copy);` - Free command copy in pipe
  - **handle_multi_pipe():** `free(args);` - Free args in pipe
  - **handle_multi_pipe():** `free(pipes[i]);` - Free individual pipe
  - **handle_multi_pipe():** `free(pipes);` - Free pipe array
  - **shell_functionality():** `free(command_copy);` - Free command copy
  - **shell_functionality():** `free(args);` - Free args array
  - **shell_functionality():** `free(args);` - Free args for exit
  - **shell_functionality():** `free(command_copy);` - Free command copy for exit
  - **shell_functionality():** `free(command_copy);` - Free command copy for cd
  - **shell_functionality():** `free(args);` - Free args for cd
  - **shell_functionality():** `free(args);` - Free args for ret
  - **shell_functionality():** `free(command_copy);` - Free command copy for ret
  - **shell_functionality():** `free(args);` - Free args after command execution
  - **shell_functionality():** `free(command_copy);` - Free command copy after execution

#### `getenv()`

- **Definition:** `char *getenv(const char *name)`
- **Usage in Code:** Getting environment variable values
- **Examples in Code:**
  - **handle_cd():** `const char *target = args[1] ? args[1] : getenv("HOME");` - Get HOME for cd
  - **handle_cd():** `const char *home = getenv("HOME");` - For tilde expansion

#### `exit()`

- **Definition:** `void exit(int status)`
- **Usage in Code:** Terminating programs
- **Examples in Code:**
  - **handle_multi_pipe():** `exit(1);` - Child process error exit in pipe
  - **shell_functionality():** `exit(1);` - Child process error exit in main
  - **shell_functionality():** `return EXIT_SUCCESS;` - Normal program termination
  - **shell_functionality():** `return EXIT_FAILURE;` - Error termination

---

### unistd.h Functions

#### `getcwd()`

- **Definition:** `char *getcwd(char *buf, size_t size)`
- **Usage in Code:** Getting current working directory for prompt
- **Example in Code:**
  - **print_prompt():** `if (getcwd(cwd, sizeof(cwd)) != NULL)` - Get current directory
- **Purpose:** Used to display current directory in shell prompt

#### `chdir()`

- **Definition:** `int chdir(const char *path)`
- **Usage in Code:** Implementing the cd built-in command
- **Example in Code:**
  - **handle_cd():** `if (chdir(target) != 0)` - Change directory
- **Returns:** 0 on success, -1 on error

#### `fork()`

- **Definition:** `pid_t fork(void)`
- **Usage in Code:** Creating child processes for command execution
- **Examples in Code:**
  - **handle_multi_pipe():** `pid_t pid = fork();` - Fork for pipe commands
  - **shell_functionality():** `pid_t pid = fork();` - Fork for regular commands
- **Returns:**
  - Child PID in parent process
  - 0 in child process
  - -1 on error

#### `execvp()`

- **Definition:** `int execvp(const char *file, char *const argv[])`
- **Usage in Code:** Executing commands in child processes
- **Examples in Code:**
  - **handle_multi_pipe():** `execvp(args[0], args);` - Execute command in pipe
  - **shell_functionality():** `execvp(args[0], args);` - Execute regular command
- **Note:** Uses PATH environment variable to find executables

#### `pipe()`

- **Definition:** `int pipe(int pipefd[2])`
- **Usage in Code:** Creating pipes for inter-process communication
- **Example in Code:**
  - **handle_multi_pipe():** `if (pipe(pipes[i]) == -1)` - Create pipe
- **Creates:** Two file descriptors: pipefd[0] (read), pipefd[1] (write)

#### `dup2()`

- **Definition:** `int dup2(int oldfd, int newfd)`
- **Usage in Code:** Redirecting stdin/stdout for pipes
- **Examples in Code:**
  - **handle_multi_pipe():** `dup2(pipes[i - 1][0], STDIN_FILENO);` - Redirect stdin from previous pipe
  - **handle_multi_pipe():** `dup2(pipes[i][1], STDOUT_FILENO);` - Redirect stdout to next pipe

#### `close()`

- **Definition:** `int close(int fd)`
- **Usage in Code:** Closing file descriptors
- **Examples in Code:**
  - **handle_multi_pipe():** `close(pipes[j][0]);` - Close read end in child
  - **handle_multi_pipe():** `close(pipes[j][1]);` - Close write end in child
  - **handle_multi_pipe():** `close(pipes[i][0]);` - Close read end in parent
  - **handle_multi_pipe():** `close(pipes[i][1]);` - Close write end in parent

---

### string.h Functions

#### `strncpy()`

- **Definition:** `char *strncpy(char *dest, const char *src, size_t n)`
- **Usage in Code:** Safely copying strings with size limit
- **Example in Code:**
  - **print_prompt():** `strncpy(cwd_copy, cwd, PATH_MAX);` - Copy current directory path

#### `strtok_r()`

- **Definition:** `char *strtok_r(char *str, const char *delim, char **saveptr)`
- **Usage in Code:** Thread-safe string tokenization
- **Examples in Code:**
  - **print_prompt():** `char *token = strtok_r(cwd_copy, "/", &saveptr);` - Split path by '/'
  - **print_prompt():** `token = strtok_r(NULL, "/", &saveptr);` - Continue path tokenization
  - **parse_input():** `char *token = strtok_r(input_line, " ", &saveptr);` - Split by spaces
  - **parse_input():** `token = strtok_r(NULL, " ", &saveptr);` - Continue space tokenization
  - **handle_multi_pipe():** `char *token = strtok_r(input, "|", &saveptr);` - Split by '|'
  - **handle_multi_pipe():** `token = strtok_r(NULL, "|", &saveptr);` - Continue pipe tokenization
  - **shell_functionality():** `char *command = strtok_r(input_line, ";", &saveptr);` - Split by ';'
  - **shell_functionality():** `command = strtok_r(NULL, ";", &saveptr);` - Continue semicolon tokenization
  - **shell_functionality():** `command = strtok_r(NULL, ";", &saveptr);` - Continue for empty commands
  - **shell_functionality():** `command = strtok_r(NULL, ";", &saveptr);` - Continue after cd
  - **shell_functionality():** `command = strtok_r(NULL, ";", &saveptr);` - Continue after ret
  - **shell_functionality():** `command = strtok_r(NULL, ";", &saveptr);` - Continue after execution

#### `strcspn()`

- **Definition:** `size_t strcspn(const char *s1, const char *s2)`
- **Usage in Code:** Finding position of unwanted characters
- **Example in Code:**
  - **shell_functionality():** `input_line[strcspn(input_line, "\n")] = '\0';` - Remove newline character

#### `strlen()`

- **Definition:** `size_t strlen(const char *s)`
- **Usage in Code:** Getting string length
- **Example in Code:**
  - **handle_multi_pipe():** `size_t len = strlen(input);` - Check input length for pipe validation

#### `strdup()`

- **Definition:** `char *strdup(const char *s)`
- **Usage in Code:** Creating heap-allocated copies of strings
- **Examples in Code:**
  - **handle_multi_pipe():** `char *cmd_copy = strdup(commands[i]);` - Copy command for pipe execution
  - **shell_functionality():** `char *command_copy = strdup(command);` - Copy command for parsing

#### `strcmp()`

- **Definition:** `int strcmp(const char *s1, const char *s2)`
- **Usage in Code:** Comparing strings for built-in commands
- **Examples in Code:**
  - **shell_functionality():** `if (strcmp(args[0], "exit") == 0)` - Check for exit command
  - **shell_functionality():** `if (strcmp(args[0], "cd") == 0)` - Check for cd command
  - **shell_functionality():** `if (strcmp(args[0], "ret") == 0)` - Check for ret command

#### `strchr()`

- **Definition:** `char *strchr(const char *s, int c)`
- **Usage in Code:** Finding characters in strings
- **Example in Code:**
  - **shell_functionality():** `char *pipe = strchr(command, '|');` - Detect pipe character in command

#### `strerror()`

- **Definition:** `char *strerror(int errnum)`
- **Usage in Code:** Converting errno to readable error messages
- **Example in Code:**
  - **handle_cd():** `fprintf(stderr, "cd fehlgeschlagen: %s\n", strerror(errno));` - CD error message

---

### sys/wait.h Functions

#### `wait()`

- **Definition:** `pid_t wait(int *status)`
- **Usage in Code:** Waiting for any child process to terminate
- **Example in Code:**
  - **handle_multi_pipe():** `wait(&status);` - Wait for child in pipeline

#### `waitpid()`

- **Definition:** `pid_t waitpid(pid_t pid, int *status, int options)`
- **Usage in Code:** Waiting for specific child processes
- **Examples in Code:**
  - **has_children():** `pid_t result = waitpid(-1, NULL, WNOHANG);` - Non-blocking check for any child
  - **shell_functionality():** `waitpid(pid, &status, 0);` - Wait for specific child process

#### `WIFEXITED()` (Macro)

- **Definition:** Macro that returns true if child terminated normally
- **Usage in Code:** Checking if process exited normally
- **Examples in Code:**
  - **handle_multi_pipe():** `if (WIFEXITED(status)) last_status = WEXITSTATUS(status);` - Check pipe exit
  - **shell_functionality():** `last_status = WIFEXITED(status) ? WEXITSTATUS(status) : -1;` - Check regular command exit

#### `WEXITSTATUS()` (Macro)

- **Definition:** Macro that returns the exit status of terminated child
- **Usage in Code:** Getting the actual exit code
- **Examples in Code:**
  - **handle_multi_pipe():** `last_status = WEXITSTATUS(status);` - Get pipe exit status
  - **shell_functionality():** `WEXITSTATUS(status)` - Get regular command exit status

---

### signal.h Functions

#### `signal()`

- **Definition:** `void (*signal(int sig, void (*func)(int)))(int)`
- **Usage in Code:** Setting up signal handlers
- **Examples in Code:**
  - **main():** `signal(SIGINT, handle_sigint);` - Handle Ctrl+C
  - **main():** `signal(SIGHUP, handle_sighup);` - Handle SIGHUP

---

## Custom Functions

### `print_prompt()`

- **Purpose:** Displays shell prompt showing last two directory levels
- **Implementation:** Uses `getcwd()`, `strtok_r()` to parse path and display "parent/current> "
- **Special Feature:** Shows only last 2 directory levels for cleaner prompt
- **Key Operations:**
  - Gets current working directory
  - Tokenizes path by '/'
  - Displays appropriate prompt format

### `has_children()`

- **Purpose:** Checks if there are active child processes without blocking
- **Implementation:** Uses `waitpid(-1, NULL, WNOHANG)` for non-blocking check
- **Returns:** 1 if children exist, 0 otherwise
- **Key Operations:**
  - Non-blocking waitpid call
  - Logic to determine child status

### `handle_sigint()`

- **Purpose:** Signal handler for Ctrl+C (SIGINT)
- **Behavior:**
  - If no children: shows exit hint and reprints prompt
  - If children exist: just flushes output
- **Key Operations:**
  - Check if children exist
  - Handle no children case
  - Handle children exist case

### `handle_sighup()`

- **Purpose:** Displays the last command return status
- **Usage:** Called by 'ret' command or SIGHUP signal
- **Parameters:** `sig` - 0 for manual call, non-zero for signal
- **Key Operations:**
  - Manual call format
  - Signal call format

### `parse_input()`

- **Purpose:** Parses input line into array of arguments
- **Implementation:** Uses `strtok_r()` to split by spaces
- **Returns:** NULL-terminated array of string pointers
- **Memory:** Allocates memory that must be freed by caller
- **Key Operations:**
  - Allocate argument array
  - Tokenize input by spaces
  - NULL-terminate array

### `handle_multi_pipe()`

- **Purpose:** Handles pipeline commands with multiple processes
- **Implementation:**
  - Creates pipes between processes
  - Forks children for each command
  - Uses `dup2()` to redirect stdin/stdout
  - Waits for all children to complete
- **Key Operations:**
  - Parse commands by '|'
  - Create pipes
  - Fork and execute each command
  - Cleanup and wait for children

### `handle_cd()`

- **Purpose:** Implements the cd built-in command
- **Features:**
  - Supports tilde (~) expansion
  - Defaults to HOME directory if no argument
  - Sets `last_status` based on success/failure
- **Key Operations:**
  - Determine target directory
  - Handle tilde expansion
  - Change directory and set status

### `shell_functionality()`

- **Purpose:** Main shell loop function
- **Responsibilities:**
  - Reads user input
  - Parses commands separated by semicolons
  - Handles built-in commands (exit, cd, ret)
  - Executes external commands via fork/execvp
  - Manages return flags for main loop control
- **Key Operations:**
  - Read and validate input
  - Process commands loop
  - Handle pipe commands
  - Handle built-in commands
  - Handle external commands

---

## Constants and Macros

### Custom Definitions

- **`MAX_ARGS`** - Maximum number of arguments (100)
- **`MAX_LINE`** - Maximum input line length (1024)
- **`DEBUG`** - Compile-time debug flag (set to 0 to disable debug output)

### File Descriptors

- **`STDIN_FILENO`** - Standard input file descriptor (0) - Used in handle_multi_pipe()
- **`STDOUT_FILENO`** - Standard output file descriptor (1) - Used in handle_multi_pipe()
- **`STDERR_FILENO`** - Standard error file descriptor (2)

### System Constants

- **`PATH_MAX`** - Maximum path length (from limits.h) - Used in print_prompt() and handle_cd()

### waitpid() Options

- **`WNOHANG`** - Non-blocking wait option - Used in has_children()
TODO: why do we use this WNOHANG?

### Error Codes

- **`ECHILD`** - No child processes exist (errno value) - Used in has_children()
- **`EXIT_SUCCESS`** - Successful program termination (0) - Used in shell_functionality()
- **`EXIT_FAILURE`** - Program termination with error (1) - Used in shell_functionality()

---

## Global Variables

### `last_status`

- **Type:** `int`
- **Purpose:** Stores the exit status of the last executed command
- **Usage:** Updated after each command execution, displayed by 'ret' command
- **Updated in:** handle_multi_pipe(), handle_cd(), shell_functionality()
- **Displayed in:** handle_sighup()

### `child_count`

- **Type:** `volatile sig_atomic_t`
- **Purpose:** Tracks number of active child processes
- **Thread Safety:** Uses `sig_atomic_t` for signal-safe access
- **Usage:**
  - Incremented: handle_multi_pipe(), shell_functionality()
  - Decremented: has_children(), handle_multi_pipe(), shell_functionality()
  - Reset: has_children()
  - Checked: has_children(), handle_sigint()

---

## Notes

1. **Memory Management:** The code carefully manages dynamic memory allocation with corresponding `free()` calls throughout the program
2. **Signal Safety:** Uses `sig_atomic_t` for variables accessed in signal handlers
3. **Thread Safety:** Uses `strtok_r()` instead of `strtok()` for thread-safe string parsing
4. **Error Handling:** Comprehensive error checking with appropriate error messages
5. **Process Management:** Proper child process tracking and cleanup
6. **Pipe Management:** Careful handling of pipe file descriptors to avoid deadlocks
7. **Input Validation:** Checks for empty commands, invalid pipe syntax, and malformed input
8. **Built-in Commands:** Implements exit, cd (with ~ expansion), and ret commands

---
