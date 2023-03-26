#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_STRING_LENGTH 1000
#define MAX_PARAM_LENGTH 100
#define shell_builtin 1
#define executable_or_error 2

bool bgProcess;
char givenCommand[MAX_STRING_LENGTH];
FILE *logs;

// function parent_main()
//     register_child_signal(on_child_exit())
//     setup_environment()
//     shell()

void setupEnv();
void shell();
void onChildExit();

int main()
{
    signal(SIGCHLD, onChildExit);
    // logs = fopen("logs.txt", "a");
    // if (logs == NULL)
    // {
    //     printf("error in file\n");
    // }
    setupEnv();
    shell();
    return 0;
}

/*
function on_child_exit()
    reap_child_zombie()
    write_to_log_file("Child terminated")
*/

void writeLogs(pid_t pid)
{
    logs = fopen("logs.txt", "a");
    fprintf(logs, "child with ID %d exited\n", pid);
    fclose(logs);
}

void onChildExit()
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        writeLogs(pid);
    }
}

/*
function setup_environment()
    cd(Current_Working_Directory)
*/

void setupEnv()
{
    char *dir = (char *)malloc(MAX_STRING_LENGTH);
    getcwd(dir, MAX_STRING_LENGTH);
    chdir(dir);
    printf("current directory: %s\n", dir);
    free(dir);
}

void pDir()
{
    char *dir = (char *)malloc(MAX_STRING_LENGTH);
    getcwd(dir, MAX_STRING_LENGTH);
    printf("%s >>", dir);
    free(dir);
}

/*
function shell()
    do
        parse_input(read_input())
        evaluate_expression():
        switch(input_type):
            case shell_builtin:
                execute_shell_bultin();
            case executable_or_error:
                execute_command():

    while command_is_not_exit
*/

char *readCmd();
void parseIn(char *cmdString, char *cmdParams[]);
void evalExp(char *cmdParams[]);
void exeBuiltin(char *cmdParams[]);
void exeCmd(char *cmdParams[]);

void shell()
{
    bool exit = false;
    while (!exit)
    {
        // printf(">> ");
        pDir();
        char *cmdParams[MAX_PARAM_LENGTH];
        parseIn(readCmd(), cmdParams);
        evalExp(cmdParams);
        int inputType;
        if (strcmp(cmdParams[0], "export") == 0 ||
            strcmp(cmdParams[0], "cd") == 0 ||
            strcmp(cmdParams[0], "echo") == 0)
        {
            inputType = shell_builtin;
        }
        else
        {
            inputType = executable_or_error;
        }
        switch (inputType)
        {
        case shell_builtin:
            // execute_shell_bultin();
            exeBuiltin(cmdParams);
            break;
        case executable_or_error:
            // execute_command();
            exeCmd(cmdParams);
            break;

        default:
            break;
        }
    }
}

char *readCmd()
{
    char *cmdString = (char *)malloc(MAX_STRING_LENGTH);
    fgets(cmdString, MAX_STRING_LENGTH, stdin);
    cmdString[strlen(cmdString) - 1] = '\0';
    memset(givenCommand, 0, MAX_STRING_LENGTH);
    strcpy(givenCommand, cmdString);
    // printf("%s",cmdString);
    return cmdString;
}

void parseIn(char *cmdString, char *cmdParams[])
{
    bgProcess = false;
    if (cmdString[strlen(cmdString) - 1] == '&')
    {
        bgProcess = true;
        cmdString[strlen(cmdString) - 1] = '\0';
    }
    memset(cmdParams, 0, MAX_PARAM_LENGTH);
    char *token = strtok(cmdString, " ");
    if (!strcmp(token, "exit"))
    {
        exit(0);
    }
    if (!strcmp(token, "echo"))
    {
        char *temp = (char *)malloc(MAX_STRING_LENGTH);
        memset(temp, 0, MAX_STRING_LENGTH);
        strncpy(temp, cmdString, 4);
        strcat(temp, " ");
        strcat(temp, cmdString + 6);
        cmdString = temp;
        cmdString[strlen(cmdString) - 1] = '\0';
        free(temp);
    }
    else if (!strcmp(token, "cd"))
    {
        char *dir = cmdString + 3;
        cmdParams[0] = token;
        // if(strlen(cmdString)<=3){
        //     cmdParams[1]=getenv("HOME");
        //     return;
        // }
        cmdParams[1] = dir;
        // printf("%s",cmdParams[1]);

        return;
    }
    int i = 0;
    while (token != NULL)
    {
        cmdParams[i++] = token;
        token = strtok(NULL, " ");
    }
}

void evalExp(char *cmdParams[])
{
    for (int i = 0; cmdParams[i] != NULL; i++)
    {
        if (cmdParams[i][0] == '$')
        {
            cmdParams[i]++;
            cmdParams[i] = getenv(cmdParams[i]);
        }
    }
}

// function execute_shell_bultin()
//     swirch(command_type):
//         case cd:
//         case echo:
//         case export:

void exeBuiltin(char *cmdParams[])
{
    if (!strcmp(cmdParams[0], "cd"))
    {
        if (!strcmp(cmdParams[1], ""))
        {
            chdir(getenv("HOME"));
        }
        else if (cmdParams[1][0] == '~')
        {
            chdir(getenv("HOME"));
        }
        else
        {
            chdir(cmdParams[1]);
        }
        // setupEnv();
    }
    else if (!strcmp(cmdParams[0], "echo"))
    {
        system(givenCommand);
    }
    else
    { // export
        char *variable = (char *)malloc(MAX_STRING_LENGTH);
        char *value = (char *)malloc(MAX_STRING_LENGTH);
        for (int i = 1; cmdParams[i] != NULL; i++)
        {
            int index = strcspn(cmdParams[i], "=");
            strncpy(variable, cmdParams[i], index);
            strcpy(value, cmdParams[i] + index + 1);
            if (value[0] == '\"')
            {
                for (int j = i; cmdParams[j][strlen(cmdParams[j]) - 1] != '\"'; j++)
                {
                    strcat(value, " ");
                    strcat(value, cmdParams[j + 1]);
                    i++;
                }
                value[strlen(value) - 1] = '\0';
            }
            setenv(variable, value, true);
            memset(variable, 0, MAX_STRING_LENGTH);
            memset(value, 0, MAX_STRING_LENGTH);
        }
        free(variable);
        free(value);
    }
}

// function execute_command()
//     child_id = fork()
//     if child:
//         execvp(command parsed)
//         print("Error")
//         exit()
//     else if parent and foreground:
//         waitpid(child)

void exeCmd(char *cmdParams[])
{
    int status;
    pid_t childID = fork();
    if (childID == -1)
    {
        perror("fork error\n");
        // printf("error can't initialize child");
        exit(EXIT_FAILURE);
    }
    else if (childID == 0)
    {
        int exeState = execvp(cmdParams[0], cmdParams);
        if (exeState == -1)
        {
            printf("can't execute this command\n");
        }
        exit(EXIT_SUCCESS);
    }
    else
    {
        if (!bgProcess)
        {
            waitpid(childID, &status, WUNTRACED);
        }
    }
}
