#include <stdlib.h>

#include "types.h"
#include "interface.h"
#include "process_manager.h"

int main()
{
    command_t* cmd = (command_t*)malloc(sizeof(command_t));
    while(true)
    {
        wait_for_cmd(cmd);
        run_command(cmd);
        // Free up the argument's resources so we can re-use the memory
        char arg = 0;
        while (cmd->argv[arg] != NULL)
        {
            free(cmd->argv[arg++]);
        }
    }
    return 0;
}