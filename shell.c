#include<stdio.h>
#include<sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>

void printPromt();
int getCommand(char** testing ,char **shellArgv,char *inputStr, char *outputStr, int flags[]);
int checkinputFlags(char *pString[30],int flags[],int numOfArgs);
void runTheProgram(int flags[],char *pString[30], int swichCase, char* inputFile, char* outputFile);
enum state  {DONTWAIT,INPUT,OUTPUT,WAIT};
//              0       1      2     3

int main()
{
    int i, numberOfArguments ;
    int flags[3] = {0};
    int inputCheck = 0;
    char *shellArgv[30];
    char *testing[30];
    char inputSTR[1024] ={0};
    char outputSTR[1024] = {0};
    while(1)
    {
        numberOfArguments = 0;
        printPromt();
        numberOfArguments = getCommand(testing,shellArgv,inputSTR,outputSTR,flags);
        if(numberOfArguments == -1){ continue;}
        if(strcmp(shellArgv[0],"exit") == 0)
        {
            printf("\nExiting to the regular Shell..\n");
            break;
        }
        inputCheck = checkinputFlags(shellArgv, flags , numberOfArguments);
        runTheProgram(flags,shellArgv,inputCheck,inputSTR,outputSTR);
        for(i=0;i<numberOfArguments;i++)
        {
            free(shellArgv[i]);
            shellArgv[i] = NULL;
        }
        for (int j = 0; j < 1024 ; ++j) 
        {
           inputSTR[j] = 0;
           outputSTR[j] = 0;
        }
        for (int k = 0; k < 3 ; ++k) 
        {
           flags[k]=0;
        }
    }
    return 0;
}
void printPromt() {
    char name[1024]= {0};
    gethostname(name,1024);
    strcat(name,":");
    char* path=getcwd(NULL,0);
    path=(char*)realloc(path,(strlen(path)+2)*sizeof(char));
    if(!path)return;
    strcat(path," #");
    strcat(name,path);
    write(1,name,strlen(name));
    free(path);
}

int getCommand(char** testing ,char **shellArgv,char *inputStr, char *outputStr, int flags[]) {
    char **iterator= shellArgv;
    char **startOfTestingStr = testing;
    int output=-1;
    int inputTesting =-1;
    int i = 0;
    int j =0;
    int numOfArgs=0;
    char* strPoint ,  *utilStr;
    char buff = 0;
    bool inputOutpFlag = false;
    char bigBuff[1024];
    char strBackUp[1024];
    int nb ;
    while(buff != '\n')
    {
        nb = read(STDIN_FILENO ,&buff,1);
        if(nb < 0)
        {
            perror("STDIN");
            return -1;
        }
        bigBuff[i] = buff;
        if(buff == '\n' && i==0)return -1;
        if(buff == '\n')bigBuff[i]='\0';
        i++;
    }
    strcpy(strBackUp, bigBuff);
/*----------------------------------------------------*/
    strPoint = strtok(bigBuff," <");
    while(strPoint != NULL)
    {
        numOfArgs++;
        *(iterator) =  (char*) malloc((strlen(strPoint)+1)*sizeof(char));
        strcpy(*(iterator),strPoint);
        if(strcmp(strPoint,">")==0)
        {
            output = j+1;
            inputOutpFlag = true;
            flags[OUTPUT]=1;
        }
        iterator++;
        strPoint = strtok(NULL," <");
        j++;
    }
    if(output != -1) 
    {
        for (int k = 0; k < numOfArgs; ++k) 
        {
            if ((strcmp(shellArgv[k], ">") == 0)) 
            {
                strcpy(outputStr, shellArgv[k+1]);
                free(shellArgv[k]);
                shellArgv[k] = NULL;
                free(shellArgv[k + 1]);
                shellArgv[k + 1] = NULL;
                break;
            }
        }
    }
    *(iterator) = NULL;
/*------------------------------------------------------------------------*/
    j=0;
    utilStr = strtok(strBackUp," ");
    while(utilStr != NULL)
    {
        *(testing) =  (char*) malloc((strlen(utilStr)+1)*sizeof(char));
        strcpy(*(testing),utilStr);
        if(strcmp(utilStr, "<") == 0)
        {
            inputTesting = j+1;
            inputOutpFlag = true;
        }
        testing++;
        utilStr = strtok(NULL," ");
        j++;
    }
    *(testing)= NULL;
    if(inputOutpFlag == true)
    {
        if(inputTesting != -1)
        {
            flags[INPUT] = 1;
            strcpy(inputStr,startOfTestingStr[inputTesting]);
        }
    }
    if(output!=-1)
        return numOfArgs+1;
    return numOfArgs;
}

int checkinputFlags(char *pString[30],int flags[],int numOfArgs) {
    char **iterator = pString;
    while (*iterator != NULL)
    {
        if(strcmp(*iterator,"&")==0 )
        {
            flags[DONTWAIT] = 1;
            free(*iterator);
            *iterator = NULL;
            return DONTWAIT;
        }
        iterator++;
    }
    return WAIT;
}

void runTheProgram(int flags[],char *pString[30], int swichCase, char* inputFile, char* outputFile) {
    int status;
    int inputStatus = -1;
    int outputStatus = -1;
    char* filename=strrchr(pString[0],'/');
    if(filename)strcpy(pString[0],filename+1);
    if(strcmp(pString[0],"cd") == 0)
    {
        if(pString[1] == NULL)
			status = chdir("/home");
        else{
            if(pString[2] != NULL){
			printf("There is too many arguments\n");
            }
			else{
				status = chdir(pString[1]);
			    if(status != 0)printf("cd: %s: No such file or directory\n",pString[1]);
            }
        }
        return;
    }
    // Forking a child process
    pid_t pid = fork();
    if (pid == -1) {
        printf("\nFailed forking child..\n");
        return;
    } else if (pid == 0) {

        if(flags[INPUT] == 1){         //we are in an input case
            inputStatus=open(inputFile,O_RDONLY);
            if(inputStatus==-1) {perror("Failed opening the file:");return ;}
            if(close(0)) perror("Failed closing the file:");
            dup(inputStatus);
        }
        if(flags[OUTPUT] == 1){      //we are in an output case
            outputStatus=open(outputFile,O_RDWR|O_CREAT,0666);
            if(outputStatus==-1) {perror("Failed opening the file:");return ;}
            if(close(1)) perror("Failed closing the file:");
            dup(outputStatus);
        }
        if (execvp(pString[0], pString) < 0) {
            perror("\nCould not execute command..\n");
        }
        exit(0);
    }
    if (flags[DONTWAIT] == 0)
    {
        wait(NULL);
    }
}


