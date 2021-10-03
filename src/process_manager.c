#include "process_manager.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>

#define PS_FIND "ps -p %u -o times="
#define PS_FIND_LEN sizeof(PS_FIND) + 10
#define CMD_DEBUG_TEMPLATE "Command Recognized: %s\n"

struct p_table_entry {
    pid_t pid;                 // OS given process ID
    char cmd[MAX_LINE_LENGTH]; // Shell input for command
    bool running;              // False if process is suspended
}; typedef struct p_table_entry process_t;

process_t process_table[MAX_PT_ENTRIES]; // Table for keeping track of children's details
size_t processes = 0;                    // Number of processes in table

/**
 * Remove a process from the process table
 */
void remove_process(pid_t pid)
{
    bool found = false;
    for (size_t i = 0; i < processes; i++)
    {
        if (found)
        {
            memcpy(&(process_table[i - 1]), &(process_table[i]), sizeof(process_t));
        }
        else if (pid == process_table[i].pid)
        {
            found = true;
        }
    }
    if (found)
    {
        processes--;
    }
}

/**
 * Get the process with the matching information from the table
 * Returns NULL if none found
 */
process_t* get_process(pid_t pid)
{
    for (size_t i = 0; i < processes; i++)
    {
        if (pid == process_table[i].pid)
        {
            return &(process_table[i]);
        }
    }
    return NULL;
}

/**
 * Spawn a new processes with the specified command details
 */
void spawn_process(command_t* cmd)
{
    if (processes >= MAX_PT_ENTRIES)
    {
        fprintf(stderr, "Unable to spawn another process (too many processes active)");
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
            perror(NULL);
        }
        _Exit(1);
    }
    else if (pid != -1) // Parent process
    {
        // Update the process lists to reflect the new process
        strcpy(process_table[processes].cmd, cmd->input);
        process_table[processes].pid = pid;
        process_table[processes].running = true;
        processes++;

        if (!cmd->background)
        {
            if (wait(NULL) == -1)
            {
                perror(NULL);
            }
            else
            {
                processes--;
            }
        }
    }
}

/**
 * Prints completed processes resource usage
 */
void print_rusage(void)
{
    struct rusage resources;
    if (getrusage(RUSAGE_CHILDREN, &resources) == -1)
    {
        perror(NULL);
    }

    printf("User time = %6ld seconds\n", resources.ru_utime.tv_sec);
    printf("Sys  time = %6ld seconds\n", resources.ru_stime.tv_sec);
}

/**
 * Print out information for all running jobs
 */
void get_jobs()
{
    printf("Running processes:\n");
    if (processes > 0)
    {
        printf(" #      PID S SEC COMMAND\n");
        for (size_t i = 0; i < processes; i++)
        {
            dbprintf("Made it to the first iteration");

            char psfind[PS_FIND_LEN];
            unsigned int rtime;
            dbprintf("Allocated stuff");

            sprintf(psfind, PS_FIND, process_table[i].pid);
            dbprintf("Finding time using %s\n", psfind);

            FILE* pspipe = popen(psfind, "r");
            fscanf(pspipe, "%u", &rtime);
            pclose(pspipe);
            dbprintf("Got rtime %u\n", rtime);

            printf("%2lu: %7u %c %3u %s", i, process_table[i].pid, process_table[i].running ? 'R': 'S', rtime, process_table[i].cmd);
        }
    }
    printf("Processes = %6ld active\n", processes);
    printf("Completed processes:\n");
    print_rusage();
}

/**
 * Send the suspect signal to the process with the given pid
 */
void suspend_job(pid_t pid)
{
    if (kill(pid, SIGSTOP) == 0)
    {
        process_t* p = get_process(pid);
        if (p != NULL)
        {
            p->running = false;
        }
    }
    else
    {
        perror(NULL);
    }
}

/**
 * Send the resume signal to the process with the given pid
 */
void resume_job(pid_t pid)
{
    if (kill(pid, SIGCONT) == 0)
    {
        process_t* p = get_process(pid);
        if (p != NULL)
        {
            p->running = true;
        }
    }
    else
    {
        perror(NULL);
    }
    
}

/**
 * Wait on the process with the given pid
 */
void wait_on_job(pid_t pid)
{
    if (waitpid(pid, NULL, 0) > -1)
    {
        remove_process(pid);
    }
    else
    {
        perror(NULL);
    }
}

/**
 * Kill (terminate) a given job and wait for it to close
 */
void kill_job(pid_t pid)
{
    if (kill(pid, SIGKILL) == 0)
    {
        wait_on_job(pid);
    }
    else
    {
        perror(NULL);
    }
}

/**
 * Wait on every running background process
 */
void wait_on_all_jobs()
{
    while (processes > 0)
    {
        wait_on_job(process_table[processes - 1].pid);
    }
}

/**
 * See header file for details
 */
void run_command(command_t* cmd)
{
    int arg1;
    if (strcmp(cmd->cmd, "exit") == 0)
    {
        dbprintf(CMD_DEBUG_TEMPLATE, "exit");
        wait_on_all_jobs();
        printf("Resources used\n");
        print_rusage();
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