# smallsh

A custom Unix shell written in C for an operating systems course portfolio project. The shell supports interactive command execution, built-in commands, redirection, foreground/background processes, and signal handling.

## Features

- Command prompt with `:`
- Blank line and comment handling
- Built-in commands:
  - `exit`
  - `cd`
  - `status`
- External command execution using `fork()` and `exec()`
- Input and output redirection with `dup2()`
- Foreground and background process support
- Custom handling for `SIGINT` and `SIGTSTP`
- Background process status reporting

## Example Usage

```bash
: ls
: cd /home/user
: status
: sleep 5 &
: exit
```

## Implementation Notes

- Parses command lines up to 2048 characters with up to 512 arguments.
- Uses `execvp()` to search the `PATH` for executable programs.
- Supports foreground-only mode toggle with `SIGTSTP`.
- Tracks exit status and terminating signals for foreground processes.
