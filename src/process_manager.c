#include "process_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>

#define PS_FIND "ps -p %u -o time="
#define CMD_DEBUG_TEMPLATE "Command Recognized: %s\n"

struct p_table_entry {
    pid_t pid;
    char cmd[MAX_LINE_LENGTH];
    bool running;
}; typedef struct p_table_entry process_t;

process_t process_table[MAX_PT_ENTRIES];
char processes = 0;

process_t* get_process(pid_t pid)
{
    for (char i = 0; i < processes; i++)
    {
        if (pid == process_table[i].pid)
        {
            return &(process_table[i]);
        }
    }
    return NULL;
}

void run_command(command_t* cmd)
{
    int arg1;
    if (strcmp(cmd->cmd, "exit") == 0)
    {
        dbprintf(CMD_DEBUG_TEMPLATE, "exit");
        _Exit(0);
    }
    else if (strcmp(cmd->cmd, "wait") == 0)
    {
        arg1 = atoi(cmd->argv[1]);
        dbprintf(CMD_DEBUG_TEMPLATE, "wait");
        wait_on_job(arg1);
    }
    else if (strcmp(cmd->cmd, "resume") == 0)
    {
        arg1 = atoi(cmd->argv[1]);
        dbprintf(CMD_DEBUG_TEMPLATE, "resume");
        resume_job(arg1);
    }
    else if (strcmp(cmd->cmd, "suspend") == 0)
    {
        arg1 = atoi(cmd->argv[1]);
        dbprintf(CMD_DEBUG_TEMPLATE, "suspend");
        suspend_job(arg1);
    }
    else if (strcmp(cmd->cmd, "kill") == 0)
    {
        arg1 = atoi(cmd->argv[1]);
        dbprintf(CMD_DEBUG_TEMPLATE, "kill");
        kill_job(arg1);
    }
    else if (strcmp(cmd->cmd, "jobs") == 0)
    {
        dbprintf(CMD_DEBUG_TEMPLATE, "jobs");
        get_jobs();
    }
    else if (strcmp(cmd->cmd, "sleep") == 0)
    {
        arg1 = atoi(cmd->argv[1]);
        dbprintf(CMD_DEBUG_TEMPLATE, "sleep");
        sleep(arg1);
    }
    else
    {
        dbprintf(CMD_DEBUG_TEMPLATE, "{CUSTOM_COMMAND}");
        spawn_process(cmd);
    }
}

void spawn_process(command_t* cmd)
{
    if (processes == MAX_PT_ENTRIES)
    {
        // TODO: handle error
    }

    pid_t pid = fork();

    if (pid == 0) // Child process
    {
        if (cmd->redir_out)
        {
            freopen(cmd->target_out, "w", stdout);
        }
        if (cmd->redir_in)
        {
            freopen(cmd->target_in, "r", stdin);
        }
        if (cmd->background && !cmd->redir_in)
        {
            // This prevents the parent's access to stdin from breaking
            // In a real shell this would be implemented differently but...
            // thats a lot of work that isn't in the assignment spec so TO NULL IT GOES
            freopen("/dev/null", "r", stdin);
        }
        if (execvp(cmd->cmd, cmd->argv) < 0)
        {
            perror("<ERROR (execvp)>");
        }
    }
    else if (pid != -1) // Parent process
    {
        // Update the process lists to reflect the new process
        processes++;
        strcpy(process_table[processes].cmd, cmd->cmd);
        process_table[processes].pid = pid;
        process_table[processes].running = true;

        if (!cmd->background)
        {
            wait(NULL);
            processes--;
        }
    }
}

void get_jobs()
{
    printf("Running processes:\n");
    if (processes > 0)
    {
        printf(" #    PID S SEC COMMAND");
        for (char i = 0; i < processes; i++)
        {
            printf("%2u: %5u %c %3u %s\n", i, process_table[i].pid, process_table[i].running ? 'R': 'S', 0, process_table[i].cmd);
        }
    }
    printf("Processes = %6d active\n", processes);
    printf("Completed processes:\n");

    struct rusage resources;
    getrusage(RUSAGE_SELF, &resources);

    printf("User time = %6d seconds\n", resources.ru_utime);
    printf("Sys  time = %6d seconds\n", resources.ru_stime);
}

void kill_job(pid_t pid)
{
    kill(pid, SIGKILL);
    wait_on_job(pid);
}

void suspend_job(pid_t pid)
{
    kill(pid, SIGSTOP);
    get_process(pid)->running = false;
}

void resume_job(pid_t pid)
{
    kill(pid, SIGCONT);
    get_process(pid)->running = true;
}

void wait_on_job(pid_t pid)
{
    waitpid(pid, NULL, WNOHANG);
}
