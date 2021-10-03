#include "interface.h"

#include <stdio.h>
#include <string.h>

#define PROMPT_TEXT "SHELL379: "
#define DELIM " \n"

/**
 * Prompt the user
 */
void prompt_user(void) {
    printf(PROMPT_TEXT);
}

/**
 * Parse a string to get a command out of it
 */
void parse_cmd(char* input, command_t* cmd) {
    // Copy input
    strcpy(cmd->input, input);

    // Begin parsing the line by whitespace
    char *found = strtok(input, DELIM);
    size_t arg = 0;

    dbprintf("Recieved: %s\n", input);

    if (found != NULL) {
        // Copy the command
        strcpy(cmd->cmd, found);

        // Copy the command to the first spot in the argument list
        cmd->argv[arg] = malloc(MAX_ARG_LENGTH + 1);
        strcpy(cmd->argv[arg], found);

        // Initialize special flages
        cmd->redir_in = false;
        cmd->redir_out = false;
        cmd->background = false;

        dbprintf("Parsed command:\n");
        dbprintf("cmd: %s\n", cmd->cmd);
        dbprintf("argv: %s, ", cmd->argv[arg]);

        // Begin getting arguments
        found = strtok(NULL, DELIM);
        while (found != NULL && arg < MAX_ARGS)
        {
            // Check i/o redirect flags, if none handle normally
            if (found[0] == '>') // Output
            {
                strcpy(cmd->target_out, found + 1);
                cmd->redir_out = true;
            }
            else if (found[0] == '<') // Input
            {
                strcpy(cmd->target_in, found + 1);
                cmd->redir_in = true;
            }
            else // Regular argument
            {
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

        // Execv expects a NULL terminated char* array
        cmd->argv[++arg] = NULL;
        dlprintf("{NULL}\n");

        // Debug purposes only
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
}

/**
 * See header file for documentation
 */
void wait_for_cmd(command_t* output) {
    prompt_user();

    char input[MAX_LINE_LENGTH];
    fgets(input, MAX_LINE_LENGTH, stdin);

    parse_cmd(input, output);
}

