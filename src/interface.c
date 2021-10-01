#include "interface.h"

#include <stdio.h>
#include <string.h>

#define PROMPT_TEXT "SHELL379: "
#define DELIM " \r\n"

void prompt_user() {
    printf(PROMPT_TEXT);
}

/**
 * 
 */
void parse_cmd(char* input, command_t* cmd) {
    char *found = strtok(input, DELIM);
    char arg = 0;

    dprintf("Recieved: %s\n", input);

    if (found != NULL) {
        // Parse cmd
        strcpy(cmd->cmd, found);

        cmd->argv[arg] = malloc(MAX_ARG_LENGTH + 1);
        strcpy(cmd->argv[arg], found);

        cmd->redir_in = false;
        cmd->redir_out = false;

        dbprintf("Parsed command:\n");
        dbprintf("cmd: %s\n", cmd->cmd);
        dbprintf("argv: %s, ", cmd->argv[arg]);

        // Begin getting arguments
        found = strtok(NULL, DELIM);
        while (found != NULL && arg < MAX_ARGS) {
            

            // Check i/o redirect flags
            if (found[0] == '>') {
                strcpy(cmd->target_out, found + 1);
                cmd->redir_out = true;
            } else if (found[0] == '<') {
                strcpy(cmd->target_in, found + 1);
                cmd->redir_in = true;
            } else {
                cmd->argv[++arg] = malloc(MAX_ARG_LENGTH + 1);
                strcpy(cmd->argv[arg], found);
                dlprintf("%s, ", cmd->argv[arg]);
            }

            found = strtok(NULL, DELIM);
        }

        // Check for background process flag
        if (strcmp(cmd->argv[arg], "&") == 0) {
            cmd->background = true;
            free(cmd->argv[arg]);
            arg--;
        }

        cmd->argv[++arg] = NULL;
        dlprintf("{NULL}\n");

        if (cmd->redir_in)
        {
            dbprintf("target_in: %s\n", cmd->target_in);
        }
        if (cmd->redir_out)
        {
            dbprintf("target_out: %s\n", cmd->target_out);
        }
        dbprintf("background: %d\n", cmd->background);

    }
    return cmd;
}

void wait_for_cmd(command_t* output) {
    prompt_user();

    char input[MAX_LINE_LENGTH];
    fgets(input, sizeof input, stdin);

    parse_cmd(input, output);
}

