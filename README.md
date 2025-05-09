# Wits Shell Tester

A minimal Unix shell implemented in C for educational purposes at the University of the Witwatersrand. This shell supports basic command execution, piping, and input/output redirection, and is designed to be tested using a suite of automated test scripts.

## 📁 Project Structure

- `witsshell.c` – Main source file implementing the custom shell.

## ⚙️ Features

- Execute standard shell commands (e.g., `ls`, `cat`, `grep`)
- Support for:
  - Piping (`|`)
  - Input redirection (`<`)
  - Output redirection (`>`)
- Basic error handling and process control using `fork`, `exec`, and `wait`.

## 🛠️ Compilation

To compile the shell:

```bash
gcc -o witsshell witsshell.c
