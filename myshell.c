#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#define BUFFER_SIZE 100

typedef struct History {
    char* name;
    struct History* next;
    int index;
} History;

History* setNewNode(char* name);
void insertHistory(History** link, History* newNode);
void printHistory(History* head);
void freeHistory(History* command);
void freeCommand(History* command);

int main(void)
{
    close(2);
    dup(1);
    char command[BUFFER_SIZE];
    
    History* head = NULL;
    History* tail = NULL;
    const char delimiters[3] = " \n";
    char* pch = NULL;
    int argsIndex = 0;
    int commandIndex = 0;
    char* args[BUFFER_SIZE];
    int isBackround = 0;
    pid_t pid = 0;
    int wstatus;
    History* temp;
    int check = 0;
    char* commandCopy;

    while (1)
    {
        fprintf(stdout, "my-shell> ");
        memset(command, 0, BUFFER_SIZE);
        fgets(command, BUFFER_SIZE, stdin);
        if (strncmp(command, "exit", 4) == 0)
        {
            break;
        }
        //=======================================//
        
        // split str into tokens
        commandCopy = command;
        pch = strtok(command, delimiters);
        while (pch)
        {
            if (strncmp(pch, "&", 1) != 0)// if you are not & so you  are an argument
            {
                args[argsIndex] = pch;
                argsIndex++;
            }
            else {
                isBackround = 1;
            }

            pch = strtok(NULL, delimiters);
        }
        
        args[argsIndex] = (char*)NULL; // marks the end of args Array
        
        check = strncmp(command, "history", 7);
        printf("check: %d\n", check);
        if (check == 0) /* if the user input "history", print the previous order */
            printHistory(head);

        // execute process
        pid = fork();
        if (pid == 0)
        {
            if (check != 0) 
            {
                if (execvp(command, args) < 0)
                {
                    perror("error");
                    exit(1);
                }

                if (head == NULL)
                {
                    head = tail;
                }

                temp = setNewNode(commandCopy);
                insertHistory(&tail, temp);
            }
        }
        else if (pid < 0)
        {
            // forking child process failed
            perror("error");
            exit(1);
        }
        else
        {
            // check if backround command
            if (isBackround == 0)
            {
                waitpid(pid, &wstatus, WUNTRACED);
            }
            isBackround = 0;
            argsIndex = 0;
        }
    }
    
    return 0;
}


History* setNewNode(char* name)
{
    History* newNode = (History*)malloc(sizeof(History));
    newNode->name = (char*)malloc(strlen(name) * sizeof(char));
    strcpy(newNode->name, name);
    newNode->next = NULL;
    return newNode;
}

// insert new command
void insertHistory(History** tail, History* newNode)
{
    newNode->next = *tail;
    *tail = newNode;
}

// print list of commands
void printHistory(History* head)
{
    while (head != NULL)
    {
        printf("%s\n", head->name);
        head = head->next;
    }
}

void freeCommand(History* command)
{
    free(command->name);
    free(command);
    return;
}

void freeHistory(History* command) {

    if (command == NULL)
        return;

    else {
        freeHistory(command->next);
        free(command->name);
        free(command);
        return;
    }
}
