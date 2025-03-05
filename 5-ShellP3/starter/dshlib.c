#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include "dshlib.h"

/* Global variable to store last child's exit code, used by 'rc' built-in */
int last_exit_code = 0;

/*
 * parse_input:
 *   Splits a command string (with no pipe) into tokens,
 *   respecting double quotes to preserve internal spaces.
 */
void parse_input(char *input, cmd_buff_t *cmd) {
    cmd->argc = 0;
    cmd->_cmd_buffer = strdup(input);
    if (!cmd->_cmd_buffer) {
        fprintf(stderr, "Memory allocation error\n");
        exit(ERR_MEMORY);
    }
    char *p = cmd->_cmd_buffer;
    while (*p) {
        // Skip leading whitespace
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        if (!*p) break;

        if (*p == '"') {
            // Quoted token
            p++;
            char *start = p;
            while (*p && *p != '"') {
                p++;
            }
            if (*p == '"') {
                *p = '\0';
                p++;
            }
            cmd->argv[cmd->argc++] = start;
        } else {
            // Non-quoted token
            char *start = p;
            while (*p && !isspace((unsigned char)*p)) {
                p++;
            }
            if (*p) {
                *p = '\0';
                p++;
            }
            cmd->argv[cmd->argc++] = start;
        }
        if (cmd->argc >= CMD_ARGV_MAX - 1) {
            break;
        }
    }
    cmd->argv[cmd->argc] = NULL;
}

/*
 * build_cmd_list:
 *   Splits `cmd_line` by '|' into multiple commands in `clist`.
 *   (If you have redirection, parse it here; otherwise skip.)
 */
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    // Example: no advanced redirection logic here; just pipe splitting.
    clist->num = 0;
    char *saveptr;
    char *segment = strtok_r(cmd_line, "|", &saveptr);
    while (segment && clist->num < CMD_MAX) {
        // Trim whitespace
        while (isspace((unsigned char)*segment)) segment++;
        if (!*segment) {
            segment = strtok_r(NULL, "|", &saveptr);
            continue;
        }
        // parse
        parse_input(segment, &clist->commands[clist->num].buff);
        clist->num++;
        segment = strtok_r(NULL, "|", &saveptr);
    }
    return 0;
}

/*
 * exec_local_cmd_loop:
 *   Main shell loop:
 *   - Reads input, checks for built-ins (cd, rc, exit).
 *   - Splits commands by '|', executes them in a pipeline if more than 1.
 *   - Single external commands run via fork/execvp.
 *   - Sets last_exit_code for the 'rc' built-in.
 */
int exec_local_cmd_loop(void) {
    char input[ARG_MAX];
    bool interactive = isatty(STDIN_FILENO);
    if (interactive) {
        printf("%s", SH_PROMPT); 
        fflush(stdout);
    }

    while (fgets(input, sizeof(input), stdin)) {
        // Remove trailing newline
        input[strcspn(input, "\n")] = '\0';

        // If empty
        if (!*input) {
            printf("warning: no commands provided\n");
            // No extra newline before the next prompt (FIX #1)
            printf("%s", SH_PROMPT);
            fflush(stdout);
            continue;
        }

        // Check exit
        if (strcmp(input, EXIT_CMD) == 0) {
            break;
        }

        // Build list of commands if there's a pipe
        command_list_t clist;
        build_cmd_list(input, &clist);

        // If exactly one command, check for built-ins
        if (clist.num == 1) {
            cmd_buff_t *cmd = &clist.commands[0].buff;
            if (cmd->argc > 0) {
                // Built-in cd
                if (strcmp(cmd->argv[0], "cd") == 0) {
                    if (cmd->argc == 1) {
                        // do nothing
                    } else if (cmd->argc == 2) {
                        if (chdir(cmd->argv[1]) != 0) {
                            perror("cd");
                        }
                    } else {
                        fprintf(stderr, "cd: too many arguments\n");
                    }
                    free(cmd->_cmd_buffer);
                }
                // Built-in rc
                else if (strcmp(cmd->argv[0], "rc") == 0) {
                    // Print last exit code
                    printf("%d\n", last_exit_code);
                    free(cmd->_cmd_buffer);
                }
                else {
                    // Single external command
                    pid_t pid = fork();
                    if (pid < 0) {
                        perror("fork failed");
                        free(cmd->_cmd_buffer);
                        continue;
                    } else if (pid == 0) {
                        // Child
                        execvp(cmd->argv[0], cmd->argv);
                        // If execvp fails, exit w/ errno => RC test sees 2 if ENOENT
                        int err = errno;
                        exit(err);
                    } else {
                        // Parent
                        int status;
                        waitpid(pid, &status, 0);
                        if (WIFEXITED(status)) {
                            last_exit_code = WEXITSTATUS(status);
                        } else {
                            last_exit_code = 1;
                        }
                    }
                    free(cmd->_cmd_buffer);
                }
            } else {
                // No tokens => do nothing
            }
        } 
        else {
            // Pipeline
            int in_fd = STDIN_FILENO;
            int pipe_fd[2];
            pid_t pids[CMD_MAX];

            for (int i = 0; i < clist.num; i++) {
                cmd_buff_t *cmd = &clist.commands[i].buff;
                
                // Create pipe if not last command
                if (i < clist.num - 1) {
                    if (pipe(pipe_fd) == -1) {
                        perror("pipe");
                        exit(1);
                    }
                }

                pid_t pid = fork();
                if (pid < 0) {
                    perror("fork failed");
                    exit(1);
                }
                if (pid == 0) {
                    // Child
                    if (i > 0) {
                        dup2(in_fd, STDIN_FILENO);
                        close(in_fd);
                    }
                    if (i < clist.num - 1) {
                        dup2(pipe_fd[1], STDOUT_FILENO);
                        close(pipe_fd[0]);
                        close(pipe_fd[1]);
                    }
                    execvp(cmd->argv[0], cmd->argv);
                    // If execvp fails, exit with errno
                    int err = errno;
                    exit(err);
                } else {
                    pids[i] = pid;
                    if (i > 0) {
                        close(in_fd);
                    }
                    if (i < clist.num - 1) {
                        in_fd = pipe_fd[0];
                        close(pipe_fd[1]);
                    }
                }
            }
            // Wait for all children
            for (int i = 0; i < clist.num; i++) {
                int status;
                waitpid(pids[i], &status, 0);
                if (WIFEXITED(status)) {
                    last_exit_code = WEXITSTATUS(status);
                } else {
                    last_exit_code = 1;
                }
                // free memory
                free(clist.commands[i].buff._cmd_buffer);
            }
        }

        // Print next prompt, no extra newline (FIX #1)
        printf("%s", SH_PROMPT);
        fflush(stdout);
    }

    // Final prompt on exit
    printf("%s", SH_PROMPT);
    fflush(stdout);
    return 0;  // success
}
