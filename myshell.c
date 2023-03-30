#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 100

typedef struct History_list{
    char* name;
    struct History_list* next;
    int index;
} History;

History* setNewNode(char* name, int index);
void insertHistory(History** link, History* newNode);
void printHistory(History* head);
void freeHistory(History* command);
char* reDoCommand(History* head, int index);
int checkCommand(char* command, int histoyFlag);
int getNumber(char* word);
int isDigit(int num);

int main(void)
{
    close(2);
    dup(1);
    char command[BUFFER_SIZE];
    
    History* head = NULL;
    const char delimiters[3] = " \n";
    char* pch = NULL;
    int argsIndex = 0;
    char* args[BUFFER_SIZE];
    int isBackround = 0;
    pid_t pid = 0;
    int wstatus;
    History* temp;
    int check = 0;
    char* commandCopy;
    int counter = 0;
    int histoyFlag = 0;
    char* errorCheck;

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

        check = checkCommand(command, histoyFlag);
        if (check != -1)
        {
            errorCheck = reDoCommand(head, check);
            if (strcmp(errorCheck, "error"))
            {
                strcpy(command, reDoCommand(head, check));
            }
        }

        pch = NULL;
        isBackround = 0;
        argsIndex = 0;
        check = 0;

        // split str into tokens
        commandCopy = (char*)malloc(strlen(command) * sizeof(char));
        strcpy(commandCopy, command);
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
        
        counter++;

        temp = setNewNode(commandCopy,counter);
        insertHistory(&head, temp);

        if (strncmp(command, "history", 7) == 0) /* if the user input "history", print the previous order */
        {            
            printHistory(head);
            histoyFlag = 1;
        }

        // execute process
        pid = fork();
        if (pid == 0)
        {
            if ((execvp(command, args) < 0) && (strncmp(command, "history", 7)))
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

int isDigit(int num)
{
    if ((num >= '0') && (num <= '9')) 
        return 1;
    return 0;
}

int getNumber(char* word)
{
    char* temp;
    int newNum = 0;

    for (int i = 1; i < strlen(word); i++)
    {
        if (!isDigit(word[i]))
        {
            if (i != 1)
            {
                strncpy(temp, word + 1, i);
                newNum = atoi(temp);
                return newNum;
            }
        }
    }

    return -1;
}

int checkCommand(char* command, int histoyFlag)
{
    int temp;
    if (histoyFlag && command[0] == '!')
    {
        if (command[1] == '!')
        {
            // execute !1
            return 1;
        }
        else{
            temp = getNumber(command);            
            return temp;
            
        }

    }
    return -1;
}

char* reDoCommand(History* head, int index)
{
    char* error = "error";
    while (head != NULL)
    {
        if (head->index == index)
        {
            return head->name;
        }
        
        head = head->next;
    }

    return error;
}


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
        printf("%d %s\n",head->index, head->name);
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
