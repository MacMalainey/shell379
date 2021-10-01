#pragma once
#include <stdbool.h>

#define MAX_LINE_LENGTH 100
#define MAX_ARGS          7
#define MAX_ARG_LENGTH   20

struct parsed_cmd
{
    char cmd[MAX_ARG_LENGTH];
    char* argv[MAX_ARGS + 2];

    bool redir_in;
    bool redir_out;
    char target_in[MAX_ARG_LENGTH];
    char target_out[MAX_ARG_LENGTH];

    bool background;
}; typedef struct parsed_cmd command_t;

/* DEBUG HELPER MACROS */
#define DEBUG_TAG "<DEBUG>: "

#ifdef DEBUG
#define dbprintf(...) \
    printf(DEBUG_TAG); \
    printf(__VA_ARGS__)
#define dlprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#define dlprintf(...)
#endif