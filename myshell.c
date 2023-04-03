#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 100
#define LAST_COMMAND -2

typedef struct History_list {
    char* name;
    struct History_list* next;
    int index;
} History;

History* setNewNode(char* name, int index);
void insertHistory(History** head, History* newNode);
void printHistory(History* head);
void freeHistory(History* command);
char* reDoCommand(History* head, int index);
int checkCommand(char* command, int historyFlag);
int getNumber(char* word);
int isDigit(char num);

int main(void)
{
    close(2);
    dup(1);
    char command[BUFFER_SIZE];

    History* temp;
    History* head = NULL;
    const char delimiters[8] = " \r\t\n";
    char* pch = NULL;
    char* args[BUFFER_SIZE];
    char* commandCopy;
    char* execute;
    char* errorCheck;
    int argsIndex = 0;
    int isBackground = 0;
    int wstatus;
    int check = 0;
    int counter = 0;
    int historyFlag = 0;
    int skip = 0;
    pid_t pid = 0;
    int j = 0;

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
        skip = 1;
        check = checkCommand(command, historyFlag);
        if (check != -1 && check != 0)
        {
            errorCheck = reDoCommand(head, check);

            skip = strcmp(errorCheck, "error");
            if (skip != 0)
            {
                strcpy(command, errorCheck);
            }
            else
            {
                printf("No History\n");
            }
        }

        pch = NULL;
        isBackground = 0;
        argsIndex = 0;
        check = 0;
 
        if (skip != 0)
        {
            // split str into tokens
            commandCopy = (char*)malloc(strlen(command) * sizeof(char));
            strcpy(commandCopy, command);
            pch = strtok(command, delimiters);
            execute = pch;
            while (pch)
            {
                if (strncmp(pch, "&", 1) != 0) // if you are not & so you are an argument
                {
                    args[argsIndex] = pch;
                    argsIndex++;
                }
                else if(commandCopy[0] != '&') {
                    isBackground = 1;
                }

                pch = strtok(NULL, delimiters);
            }
            args[argsIndex] = (char*)NULL;  // marks the end of args Array
            counter++;
            historyFlag = 1;
            temp = setNewNode(commandCopy, counter);
            insertHistory(&head, temp);

            if (strncmp(command, "history", 7) == 0) // if the user input "history", print the previous order
            {
                printHistory(head);
            }
            // execute process
            else {
                pid = fork();
                if (pid == 0)
                {
                    if ((execvp(execute, args) < 0))
                    {
                        perror("error");
                        exit(1);
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
                    // check if background command
                    if (isBackground == 0)
                    {
                        waitpid(pid, &wstatus, WUNTRACED);
                    }
                    isBackground = 0;
                    argsIndex = 0;
                }
            }
        }
    }

    freeHistory(head);
    return 0;
}

// func to check if digit
int isDigit(char num)
{
    if ((num >= '0') && (num <= '9'))
        return 1;     // is digit
    return 0;
}

// func to check if number (int)
int getNumber(char* word)
{
    char* temp;
    int newNum = -1;

    if (isDigit(word[1]))
    {
        newNum = atoi(word+1);
    }

    return newNum;
}

//func to check command type
int checkCommand(char* command, int historyFlag)
{
    int temp;
    if (historyFlag && command[0] == '!')
    {
        if (command[1] == '!')              // double "!!"
        {
            // execute last command input
            return LAST_COMMAND;
        }
        else {
            temp = getNumber(command);      // command number to execute
            return temp;
        }
    }
    return -1;                                   // regular command (no "!")
}

// re execution of the command
char* reDoCommand(History* head, int index)
{
    while (head != NULL)
    {
        if (index == LAST_COMMAND)
        {
            return head->name;                  // last command in History
        }
        else if (head->index == index)
        {
            return head->name;                  // wanted command
        }
        head = head->next;
    }
    return "error";                               // no such command in History
}

// set a new command node
History* setNewNode(char* name, int index)
{
    History* newNode = (History*)malloc(sizeof(History));
    if (newNode == NULL)
    {
        perror("error");
        exit(1);
    }
    newNode->name = (char*)malloc(strlen(name) * sizeof(char));

    if (newNode->name == NULL)
    {
        perror("error");
        exit(1);
    }

    strcpy(newNode->name, name);
    newNode->next = NULL;
    newNode->index = index;
    return newNode;
}

// insert new command to History
void insertHistory(History** head, History* newNode)
{
    newNode->next = *head;
    *head = newNode;
}

// print list of previous commands
void printHistory(History* head)
{
    while (head != NULL)
    {
        printf("%d \t %s", head->index, head->name);
        head = head->next;
    }
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
