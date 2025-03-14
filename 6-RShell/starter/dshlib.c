// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <ctype.h>
// #include <stdbool.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/wait.h>

// #include "dshlib.h"



// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <ctype.h>
// #include <stdbool.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/wait.h>

// #include "dshlib.h"

// /**** 
//  **** FOR REMOTE SHELL USE YOUR SOLUTION FROM SHELL PART 3 HERE
//  **** THE MAIN FUNCTION CALLS THIS ONE AS ITS ENTRY POINT TO
//  **** EXECUTE THE SHELL LOCALLY
//  ****
//  */

// /*
//  * Implement your exec_local_cmd_loop function by building a loop that prompts the 
//  * user for input.  Use the SH_PROMPT constant from dshlib.h and then
//  * use fgets to accept user input.
//  * 
//  *      while(1){
//  *        printf("%s", SH_PROMPT);
//  *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
//  *           printf("\n");
//  *           break;
//  *        }
//  *        //remove the trailing \n from cmd_buff
//  *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
//  * 
//  *        //IMPLEMENT THE REST OF THE REQUIREMENTS
//  *      }
//  * 
//  *   Also, use the constants in the dshlib.h in this code.  
//  *      SH_CMD_MAX              maximum buffer size for user input
//  *      EXIT_CMD                constant that terminates the dsh program
//  *      SH_PROMPT               the shell prompt
//  *      OK                      the command was parsed properly
//  *      WARN_NO_CMDS            the user command was empty
//  *      ERR_TOO_MANY_COMMANDS   too many pipes used
//  *      ERR_MEMORY              dynamic memory management failure
//  * 
//  *   errors returned
//  *      OK                     No error
//  *      ERR_MEMORY             Dynamic memory management failure
//  *      WARN_NO_CMDS           No commands parsed
//  *      ERR_TOO_MANY_COMMANDS  too many pipes used
//  *   
//  *   console messages
//  *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
//  *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
//  *      CMD_ERR_EXECUTE        print on execution failure of external command
//  * 
//  *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
//  *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
//  * 
//  *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
//  *      fork(), execvp(), exit(), chdir()
//  */
// int exec_local_cmd_loop()
// {

//     // THIS CODE SHOULD BE THE SAME AS PRIOR ASSIGNMENTS
   
//     return OK;
// }



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

#include "dshlib.h"

/* Keep track of last child's exit code (for 'rc' built-in if you want it) */
static int last_exit_code = 0;

/* 
 * parse_input:
 *   Splits a command string (no pipe) into tokens, respecting double quotes.
 */
static void parse_input(char *input, cmd_buff_t *cmd) {
    cmd->argc = 0;
    cmd->_cmd_buffer = strdup(input);
    if (!cmd->_cmd_buffer) {
        fprintf(stderr, "Memory allocation error\n");
        exit(ERR_MEMORY);
    }
    char *p = cmd->_cmd_buffer;

    while (*p) {
        // skip leading spaces
        while (*p && isspace((unsigned char)*p)) p++;
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
 *   Splits cmd_line by '|' into multiple commands in clist.
 *   For simple assignments, we skip redirection handling (or minimal).
 */
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    clist->num = 0;
    char *saveptr;
    char *segment = strtok_r(cmd_line, "|", &saveptr);

    while (segment && clist->num < CMD_MAX) {
        // Trim leading space
        while (isspace((unsigned char)*segment)) segment++;
        if (!*segment) {
            // skip empty segments
            segment = strtok_r(NULL, "|", &saveptr);
            continue;
        }
        parse_input(segment, &clist->commands[clist->num].buff);
        // If you want to handle infile/outfile redirection, do it here
        clist->commands[clist->num].infile  = NULL;
        clist->commands[clist->num].outfile = NULL;
        clist->commands[clist->num].append  = 0;
        clist->num++;
        segment = strtok_r(NULL, "|", &saveptr);
    }

    return 0; // success
}

/*
 * exec_local_cmd_loop:
 *   Repeatedly prompt user, parse pipeline, handle built-ins, fork/exec.
 */
int exec_local_cmd_loop(void) {
    bool interactive = isatty(STDIN_FILENO);
    char input[ARG_MAX];

    if (interactive) {
        printf("%s", SH_PROMPT);
        fflush(stdout);
    }

    while (fgets(input, sizeof(input), stdin)) {
        // strip newline
        input[strcspn(input, "\n")] = '\0';

        if (!*input) {
            fprintf(stderr, "warning: no commands provided\n");
            if (interactive) {
                printf("%s", SH_PROMPT);
                fflush(stdout);
            }
            continue;
        }

        // check for "exit"
        if (strcmp(input, EXIT_CMD) == 0) {
            break;
        }

        // build command list
        command_list_t clist;
        build_cmd_list(input, &clist);

        if (clist.num == 0) {
            fprintf(stderr, "warning: no commands provided\n");
        } else if (clist.num == 1) {
            // single command => maybe handle built-ins or external
            cmd_buff_t *cmd = &clist.commands[0].buff;
            if (cmd->argc > 0) {
                // built-in: cd
                if (strcmp(cmd->argv[0], "cd") == 0) {
                    if (cmd->argc == 2) {
                        if (chdir(cmd->argv[1]) != 0) {
                            perror("cd");
                        }
                    } else if (cmd->argc < 2) {
                        // no arg => do nothing or go HOME
                    } else {
                        fprintf(stderr, "cd: too many arguments\n");
                    }
                }
                // built-in: rc (if you want to show last exit code)
                else if (strcmp(cmd->argv[0], "rc") == 0) {
                    printf("%d\n", last_exit_code);
                }
                // otherwise external
                else {
                    pid_t pid = fork();
                    if (pid < 0) {
                        perror("fork");
                    } else if (pid == 0) {
                        execvp(cmd->argv[0], cmd->argv);
                        // if exec fails => exit with errno
                        int err = errno;
                        exit(err);
                    } else {
                        int status;
                        waitpid(pid, &status, 0);
                        if (WIFEXITED(status)) {
                            last_exit_code = WEXITSTATUS(status);
                        } else {
                            last_exit_code = 1;
                        }
                    }
                }
                // free command buffer
                free(cmd->_cmd_buffer);
            }
        } else {
            // pipeline
            int in_fd = STDIN_FILENO;
            int pipe_fd[2];
            pid_t pids[CMD_MAX];

            for (int i = 0; i < clist.num; i++) {
                cmd_buff_t *cmd = &clist.commands[i].buff;

                if (i < clist.num - 1) {
                    if (pipe(pipe_fd) < 0) {
                        perror("pipe");
                        exit(1);
                    }
                }

                pid_t pid = fork();
                if (pid < 0) {
                    perror("fork");
                    exit(1);
                }
                if (pid == 0) {
                    // child
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

            // wait for all children
            for (int i = 0; i < clist.num; i++) {
                int status;
                waitpid(pids[i], &status, 0);
                if (WIFEXITED(status)) {
                    last_exit_code = WEXITSTATUS(status);
                } else {
                    last_exit_code = 1;
                }
                // free
                free(clist.commands[i].buff._cmd_buffer);
            }
        }

        // next prompt
        if (interactive) {
            printf("%s", SH_PROMPT);
            fflush(stdout);
        }
    }

    // final prompt (optional)
    if (interactive) {
        printf("%s", SH_PROMPT);
        fflush(stdout);
    }
    return 0;
}
