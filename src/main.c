#include "types.h"
#include "interface.h"
#include "process_manager.h"

int main()
{
    command_t cmd;
    while(true)
    {
        // Get the next command and run it
        wait_for_cmd(&cmd);
        run_command(&cmd);

        // Free up the memory we allocated for each argument
        size_t arg = 0;
        while (cmd.argv[arg] != NULL)
        {
            free(cmd.argv[arg++]);
        }
    }
    return 0;
}