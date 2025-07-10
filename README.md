# Testat Betriebsystem SoSe2025

## How to Execute

To run and test this program, simply execute the provided `testing.sh` script in your terminal:

```bash
./testing.sh
```

This script will automatically build and test the shell implementation as required.

### Test Suite Overview

The provided test suite validates the core functionality of the minishell implementation. It covers:

- **Command parsing and execution**
- **Environment variable handling**
- **Error management**

After running the tests, you can use the compiled `minishell` binary for interactive use:

```bash
./minishell
```

This allows you to manually verify the shell's behavior and interact with it as a user.

## Requirements

|No. | Task | DONE | Notes |
|----|------|------|-------|
| 1 | Zu Beginn und nach jedem ausgeführten Kommando wird ein Prompt ausgegeben, der (mindestens) das aktuelle Verzeichnis und ein Trennzeichen anzeigt (z.B. /home/fd3430> ).| DONE, with some Notes | Yes, but only the last 2 actual folder. Not from the root. see the print_prompt() |
| 2 | Es werden Eingaben beliebiger im System vorhandener Kommandos entgegen genommen (implementieren Sie keine Kommandos selbst, sondern erlauben Sie lediglich die Ausführung von Kommandos wie z.B. ls, ps, dir, usw.). Dabei soll natürlich die PATH-Variable ausgewertet werden, so dass Sie einfach ls statt /usr/bin/ls eingeben können. | DONE | Yes, therefore we use execvp() to execute the programs, so tath we do not need the path variable to pass on to the execvp |
| 3 | Bei der Ausführung von Programmen soll die Nutzung von Parametern möglich sein (also nicht nur die Ausführung des Kommandos ps bzw. ls, sondern auch ps ax oder ls -a -l). | DONE, with some Notes | Yes, but to optimize the program we make sure that only 100 arguments that can be given |
| 4 | Die Shell soll blockieren (warten) bis das jeweils aufgerufene Programm terminiert. Anschließend kann ein weiteres Programm gestartet werden. | DONE | Yes, therefore, when the program is being executed, we use another process form the fork() see the shell_functionality() |
| 5 | Der Aufruf mehrere Programme in nur einer Kommandoeingabe soll möglich sein. Die Programme und deren Parameter werden dabei durch ; getrennt (z.B. ls -al ; ps ax). | DONE | Yes, int shell_functionality it is being implemented while(command) with iterator strtok_r throguh ‘;’|
| 6 | Zwei Programme sollen durch eine Pipe (Trennzeichen \|) verbunden werden können. (Mehr als zwei Programme sind nicht nötig, also nur eine einzige Pipe pro Eingabe). | DONE, with some notes | Yes, even though we impelment the multi pipeline |
| 7 |  Das Wechseln von Verzeichnissen soll möglich sein. Dies muss von Ihnen implementiert werden, da cd kein Programm im System ist, sondern eine Funktion der Shell. | DONE |see the shell_functionality() |
| 8 | Die Shell soll durch die Eingabe von exit beendet werden können. Auch dies ist kein Systemprogramm, sondern eine Funktion der Shell. | DONE | see the shell_functionality() |
| 9a | Der Rückgabewert des letzten Kommandos soll durch die Eingabe des Kommandos ret angezeigt werden. Implementieren SIe dazu das Kommando ret. | DONE | see the shell_functionality() |
| 9b |Das Signal SIGHUP soll abgefangen werden. Ein eigener Signalhandler soll ebenfals den Rückgabenwert des letzten Kommandos anzeigen. | DONE | see the handle_sighup() and the implementation in the main function |
| 10 | Das Signal SIGINT (Eingabe von Strg+C) soll abgefangen werden. Statt das Programm mittels Strg+C zu beenden, soll eine Bildschirmausgabe erfolgen, die darauf hinweist, dass das Programm durch Eingabe des Kommandos exit zu beendet ist. | DONE | see the handle_sigint() and the implementation in the main function |

## List of the functions

see the `function_documentation.md`

## Explanation: Limiting the Prompt to Two Last Folders

### Goal

The function `print_prompt()` is designed to show a custom shell prompt based on the current working directory (CWD), but instead of printing the **full path**, it prints only the **last two folder names** of the path where the shell is currently located.

---

### How the Splitting Works

```c
char *token = strtok_r(cwd_copy, "/", &saveptr);
```

This line and the following loop:

```c
while (token) {
    folders[count++] = token;
    token = strtok_r(NULL, "/", &saveptr);
}
```

* Split the current directory string (`cwd`) by the slash `/` delimiter.
* Each folder (directory) name between slashes is stored as a token in the `folders[]` array.
* The variable `count` tracks how many folder levels exist.

---

### Why Only the Last Two?

```c
if (count >= 2) {
    printf("%s/%s> ", folders[count - 2], folders[count - 1]);
}
```

* This displays only the **second-last and last** folder names as the shell prompt.
* It keeps the prompt concise and user-friendly, especially in deep directory structures.

#### Example

For a working directory like:

```bash
/home/user/projects/shell
```

The prompt would show:

```bash
projects/shell>
```

If you are in a shallow directory like `/home`, it will only show:

```bash
home>
```

And if you are in `/`, it will simply show:

```bash
/>
```

---

### Summary

* The loop with `strtok_r` splits the CWD into folder components.
* Only the last two folders are printed to make the shell prompt short and readable.
* It dynamically adapts depending on how deep the current location is.

---

## Explanation: Tokenization in `parse_input`

### What Kind of Splitting?

The function `parse_input` is responsible for **splitting a command line into individual words** (or tokens) separated by **whitespace**. These tokens typically represent:

* The command name (e.g., `ls`, `grep`, `echo`)
* The arguments or parameters passed to the command (e.g., `-l`, `file.txt`)

For example:

```bash
ls -l /home/user
```

will be split into:

```c
args[0] = "ls"
args[1] = "-l"
args[2] = "/home/user"
args[3] = NULL
```

---

### How the Splitting Happens

The function uses `strtok_r`, a thread-safe variant of `strtok`, to tokenize the input line.

#### Key Lines

```c
char *token = strtok_r(input_line, " ", &saveptr);
```

* `input_line`: The string to split
* `" "`: The delimiter (space character)
* `saveptr`: Used by `strtok_r` to keep track of the current position across successive calls

```c
while (token && i < MAX_ARGS - 1) {
    args[i++] = token;
    token = strtok_r(NULL, " ", &saveptr);
}
```

* This loop continues until:

  * No more tokens are found, or
  * The maximum number of allowed arguments (`MAX_ARGS - 1`) is reached

```c
args[i] = NULL;
```

* This ensures the returned array of strings ends with a `NULL` pointer — a requirement for functions like `execvp()` which expect a `NULL`-terminated array.

---

### What Happens Internally?

* `strtok_r` modifies the original `input_line` by inserting `\0` (null terminator) at every delimiter.
* Each token is a pointer within `input_line`. Therefore, **you must not free `input_line` before you are done using the tokens**.

---

### Summary of parse input

* `parse_input` splits an input command line into an array of arguments using space (`' '`) as the delimiter.
* It returns a dynamically allocated array of `char*` pointers.
* Each token is stored in-place inside the modified `input_line`.
* This split is essential for allowing `execvp()` to correctly execute the intended command with arguments.

---

## Explanation: The `pipe()` Function in `handle_multi_pipe`

### Purpose of `pipe()`

The `pipe()` system call in Unix/Linux is used to create a **unidirectional communication channel**—commonly used for **inter-process communication (IPC)**. In the context of a shell, it allows the output of one command to be passed as input to another, i.e., it enables piping: `ls | grep txt`.

---

### Syntax

```c
int pipe(int pipefd[2]);
```

* `pipefd[0]`: read end of the pipe
* `pipefd[1]`: write end of the pipe
* Returns 0 on success, -1 on failure

### Example implementation pipe

```c
int pipefd[2];
pipe(pipefd);  // pipefd[0] for reading, pipefd[1] for writing
```

---

### In `handle_multi_pipe()`

The goal is to support commands like:

```bash
ps aux | grep root | wc -l
```

This requires creating N-1 pipes for N commands. Here's what happens step by step:

#### 1. **Allocation of Pipes**

```c
int **pipes = malloc((count - 1) * sizeof(int *));
for (int i = 0; i < count - 1; ++i) {
    pipes[i] = malloc(2 * sizeof(int));
    pipe(pipes[i]);
}
```

* This allocates one pipe between every two commands.

#### 2. **Forking and Redirection**

In the child process:

```c
if (i > 0) {
    dup2(pipes[i - 1][0], STDIN_FILENO); // Read input from the previous command
}
if (i < count - 1) {
    dup2(pipes[i][1], STDOUT_FILENO);   // Write output to the next command
}
```

* `dup2()` redirects the input/output file descriptors to the pipe ends.
* It sets up the current command to read from the previous command's output and write to the next command’s input.

#### 3. **Closing Pipe FDs**

All file descriptors are closed in both the parent and child to avoid FD leaks and ensure correct EOF behavior.

---

### Why Multiple Pipes?

Each pipe is only used between **two adjacent commands**. For N commands:

* We need **N-1 pipes**.
* Every pipe connects output of one command to input of the next.

---

### Summary of pipe

* `pipe()` creates communication channels for IPC.
* In a shell, it links the output of one command to the input of another.
* Your implementation creates `count - 1` pipes for `count` commands split by `|`.
* Proper `dup2()` usage ensures the stdin/stdout redirection.
* `execvp()` then replaces the child process image with the actual command.

This mechanism is key to enabling classic Unix shell pipelines.

---

## shell_functionality

in the `shel_functionality()` you will see this commands within `fgets`

```c
    if (feof(stdin))
        { // > ctr + d sends EOF (End-Of-File) signal. Therefore we should use this feof() instead
            handle_sighup(1);
            clearerr(stdin); // > Needed, because otherwise the Comand Line will behave like the input is always feof. 
            //TODO explain: why this happen behind the hood? (explain in .md file)
                             // > $ man fgets >   ##Return Value## fgets() returns s on success, and NULL on error or when end of file occurs while no characters have been read.
            {
                *retFlag = 3;
                return 0; // > No need to read the input when this signal is being given, just go to the next iteration and wait until someone give an inputs!
            }; // > No need to read the input when this signal is being given, just go to the next iteration and wait until someone give an inputs!
        }
        else
        {
            printf("[Hinweis] Unknown Inputs, exit terminal!"); // TODO explain: give an example when this happen! (in other .md file) 
            return EXIT_FAILURE;
        }
```

## FAQ

Question: Can I use fgets with STDIN_FILENO? so that I can read the input file descriptor directly?

Answer from copilot: fgets expects a FILE* stream, not a file descriptor like STDIN_FILENO. You cannot directly use STDIN_FILENO (which is an int, usually 0) with fgets. If you want to read from a file descriptor, you must use read(), not fgets(). Example using read(). But then you must manually null-terminate and handle input_line as a C string.

```c
ssize_t bytes_read = read(STDIN_FILENO, input_line, MAX_LINE - 1);
if (bytes_read <= 0) { ... }
```

> Background of this Question: within the test file: `Test(shell_functionality, MOCK_basic_command_execution)` I want to add inputs automatically, without the user add the inputs through it. The solutions is (from Wrap AI) using pipe and then write the inputs and send it into the file descriptor that has reroute thorug the dup2. see the code for more info

```c
    // Create pipes for input and output redirection
    int input_pipe[2], output_pipe[2];
    cr_assert_neq(pipe(input_pipe), -1, "Failed to create input pipe");
    cr_assert_neq(pipe(output_pipe), -1, "Failed to create output pipe");

    pid_t pid = fork();
    cr_assert_neq(pid, -1, "fork failed");
    
    if (pid == 0) {
        // Child process - this will run the shell
        
        // Initialize signal handlers
        signal(SIGINT, handle_sigint);
        signal(SIGHUP, handle_sighup);
        
        // Redirect stdin to read from input_pipe
        close(input_pipe[1]); // Close write end of input pipe
        dup2(input_pipe[0], STDIN_FILENO);
        close(input_pipe[0]);
        
        // Redirect stdout to write to output_pipe
        close(output_pipe[0]); // Close read end of output pipe
        dup2(output_pipe[1], STDOUT_FILENO);
        close(output_pipe[1]);
        
        // Run shell functionality once
        for (int i = 0; i < 2; i++) {
            int retFlag = 0;
            shell_functionality(&retFlag);    
        }
        
        exit(0);
    } else {
        // Parent process - this will provide input and capture output
        
        // Close unused pipe ends
        close(input_pipe[0]);
        close(output_pipe[1]);
        
        // Send test command to shell
        const char *test_command = "sleep 3\n";
        write(input_pipe[1], test_command, strlen(test_command));
        
        ...

        // Send another command to ensure shell continues
        const char *next_command = "echo hallo\n";
        write(input_pipe[1], next_command, strlen(next_command));

        // After sending the commands, we can close the input pipe
        
        // Send EOF to signal end of input
        close(input_pipe[1]); // Close input pipe to signal EOF
        ...
    }
```

---
is it okay that the design of the prompt like this! (see the `print_prompt();`)?
Answer Hr. Sven R.: No worries, that is okay

---

Question: uniderektional pipe means that only one process can send to another and the other cannot send the result?
My Answer: Yes, pipe is not biderectional. therefor for other process to send another message to the one process it needs another pipes! (see the Ueubung 6)
