# Unix Shell

wish is a simple shell implementation written in C, based on the OSTEP "processes-shell" project. It supports built-in commands, command execution via a search path, output redirection, and parallel command execution.

## Features

- Executes commands from a defined search path (e.g., `/bin`)
- **Built-in Commands:**
  - `exit`: Terminates the shell
  - `cd [directory]`: Changes the current working directory
  - `path [path1] [path2] ...`: Replaces the shell's search path. An empty path command clears the path
- **Output Redirection:** Supports redirecting both stdout and stderr of a command to a file using the `>` operator (e.g., `ls -l > out.txt`)
- **Parallel Commands:** Supports running multiple commands in parallel, separated by the `&` operator (e.g., `cmd1 & cmd2`). The shell waits for all parallel commands to finish before prompting for new input

## Getting Started

### 1. Compiling

The shell is contained in a single `wish.c` file. To compile it, use gcc:

```bash
gcc wish.c -o wish
```

### 2. Runnig the shell

After compiling, you can run the shell in its interactive mode:

```bash
./wish
```
This will start the wish> prompt.


You can also run the shell in batch mode by providing a file containing commands as an argument:
```bash
./wish tests/01.run
```

### 3. Runnig Tests
A test script `test-wish.sh` is provided to verify all functionality. To run the automated test suite:

```bash
./test-wish.sh
```
