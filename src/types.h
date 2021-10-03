#pragma once
#include <stdbool.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 101
#define MAX_ARGS          8
#define MAX_ARG_LENGTH   21
#define MAX_PT_ENTRIES   33

struct parsed_cmd
{
    char cmd[MAX_ARG_LENGTH];    // The given command (without arguments)
    char* argv[MAX_ARGS + 2];    // The arguments provided
    char input[MAX_LINE_LENGTH]; // The original input

    bool redir_in;                   // True if target_in should be used to redirect input
    bool redir_out;                  // True if target_out should be used to redirect output
    char target_in[MAX_ARG_LENGTH];  // Target file for redirected input
    char target_out[MAX_ARG_LENGTH]; // Target file for redirected output

    bool background;                 // Run process in background or wait before getting other input
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