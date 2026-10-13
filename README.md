<div align="center">

# 🐚 EShell

**A Unix shell built from scratch in C** — command parsing, pipes, built-in commands, and process management, implemented directly on top of low-level system calls.

[![Language](https://img.shields.io/badge/language-C-00599C?style=flat-square&logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/platform-Linux-FCC624?style=flat-square&logo=linux&logoColor=black)](https://www.linux.org/)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue?style=flat-square)](https://github.com/emilndersen/-Eshell/tree/main#)

</div>

```
**********************************************

           ********** EShell ************

      "Meow Meow Meow Meow Meow Meow Meow Meow"

**********************************************

eshell:/home/user/projects $ ls | grep .c
main.c
```

## 📖 About

A hands-on learning project: a bash-like command interpreter written entirely from scratch, with no shell libraries — just raw Unix system calls. The goal was to actually understand how a shell works under the hood: process creation, inter-process communication through pipes, and file descriptor management.

## 🛠️ Built with

| Category | Tools / Calls |
|---|---|
| Language | C |
| Compiler | GCC |
| Line editing & history | [GNU Readline](https://tiswww.case.edu/php/chet/readline/rltop.html) |
| Process control | `fork()`, `execvp()`, `wait()` / `waitpid()` |
| IPC (pipes) | `pipe()`, `dup2()` |
| Filesystem | `chdir()`, `getcwd()` |
| String handling | `strtok()`, `strchr()`, `strcmp()` |
| Dev environment | Linux / WSL2 |

## ✨ Features

- **Line editing & history** — built on GNU Readline; arrow keys recall previous commands
- **Command parsing** — input is tokenized into a command plus arguments via `strtok`
- **Pipes** — `command1 | command2` connects one process's output to another's input via `pipe()` and two `fork()` calls
- **Built-in commands** — `cd` and `exit` run inside the shell process itself (not as child processes), so directory changes actually persist between commands
- **External commands** — everything else runs through `fork()` + `execvp()`, inheriting the real environment (network, filesystem, permissions) — `grep`, `ls`, `sudo apt update`, etc. all work exactly as they would in bash
- **Dynamic prompt** — shows the current working directory, refreshed via `getcwd()` on every loop iteration

## 🚀 Build & Run

Requires `gcc` and the GNU Readline development headers.

```bash
sudo apt-get install libreadline-dev
gcc main.c -o shell -lreadline
./shell
```

Type `exit` or press `Ctrl+D` on an empty line to quit.

## ⚙️ How it works

Each loop iteration:

1. `readline()` displays the prompt and reads a line of input
2. Empty input is skipped; everything else is added to history
3. The line is checked for a pipe character (`|`) — if found, it's split into two commands and handed to `execute_pipe()`
4. Otherwise, the line is tokenized into an argument array
5. Built-in commands (`cd`, `exit`) are checked first and handled directly, without spawning a process
6. Anything else runs via `fork()` + `execvp()`, with the parent waiting for the child via `wait()` before reading the next command

**Pipes in detail:** `execute_pipe()` creates a kernel pipe with `pipe()`, then forks twice — the first child redirects its `stdout` to the pipe's write end and runs the left-hand command, the second redirects its `stdin` to the pipe's read end and runs the right-hand command. The parent closes both ends of the pipe (required, or the second process never sees EOF and hangs) and waits for both children with `waitpid()`.

## 🚧 Known limitations

This is a learning project, not a production shell. Not yet supported:

- Command chaining (`&&`, `;`, `||`)
- I/O redirection (`>`, `<`, `>>`)
- Multi-stage pipes (`a | b | c`) — currently only a single pipe is supported
- Quoted arguments (splitting still happens on `|` regardless of quotes)
- Tilde expansion (`cd ~`) — use absolute or relative paths instead
- Background jobs (`&`)

## 🎯 What this project covers

Built as a practical way to relearn C and systems programming fundamentals: pointers, string tokenization, manual memory management, process creation and control, file descriptors, and inter-process communication.
