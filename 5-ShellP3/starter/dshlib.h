#ifndef __DSHLIB_H__
#define __DSHLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Maximum sizes for strings and arrays
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8         // Maximum number of piped commands
#define CMD_ARGV_MAX 32   // Maximum tokens per command

// Shell prompt and built-in command name (updated for ShellP3)
#define EXIT_CMD "exit"
#define SH_PROMPT "dsh3> "

// Return codes
#define OK 0
#define WARN_NO_CMDS -1
#define ERR_TOO_MANY_COMMANDS -2
#define ERR_MEMORY -3

// Console messages
#define CMD_WARN_NO_CMD "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT "error: piping limited to %d commands\n"
#define CMD_ERR_EXECUTE "error: execution failure\n"

// Structure for a single command buffer (for non-piped commands)
typedef struct cmd_buff {
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;  // dynamically allocated copy of the input
} cmd_buff_t;

/* Structures for piped commands and redirection support */
typedef struct command {
    cmd_buff_t buff;    // parsed command tokens (redirection tokens removed)
    char *infile;       // input redirection file (if any)
    char *outfile;      // output redirection file (if any)
    int append;         // 1 if output should be appended (>>), 0 if overwritten (>)
} command_t;

typedef struct command_list {
    int num;
    command_t commands[CMD_MAX];
} command_list_t;

/* Function prototypes */
int exec_local_cmd_loop(void);
void print_dragon(void);

/* Parser for building a list of commands (for pipes and redirection) */
int build_cmd_list(char *cmd_line, command_list_t *clist);

#endif
