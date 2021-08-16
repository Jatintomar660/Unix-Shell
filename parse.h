#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#define executable "./072.out"
#define splitters " \t\r\n\a\v"
#define num 120
#define minNum 60
#define somin 18
char replacer = 128;
char PipRep = 130;
char InRep = 132;
char OutRep = 134;

// Counts the Space in string + 2
int Space_count1(char *buff)
{
    int c = 0;
    char *New = strdup(buff);
    char *tmp = strtok(New, splitters);
    while (tmp != NULL)
    {
        tmp = strtok(NULL, splitters);
        c++;
    }
    c++;
    if (New)
    {
        free(New);
        New = NULL;
    }
    return c;
}

// Counts the Space in string + 2
int Space_count_Gen(char *buff, char *g)
{
    int c = 0;
    char *New = strdup(buff);
    char *tmp = strtok(New, g);
    while (tmp != NULL)
    {
        tmp = strtok(NULL, g);
        c++;
    }
    c++;
    if (New)
    {
        free(New);
        New = NULL;
    }
    return c;
}

// Counts the Space in string + 2
int Space_count_Pipe(char *buff)
{
    int c = 0;
    char *New = strdup(buff);
    char *tmp = strtok(New, "|");
    while (tmp != NULL)
    {
        tmp = strtok(NULL, "|");
        c++;
    }
    c++;
    if (New)
    {
        free(New);
        New = NULL;
    }
    return c;
}

// Always duplicate (strdup) the string (1st argument) before using String Stok
// And Free it after usage.
void StringStok(char *buff, char **P)
{
    //int d=Space_count1(buff);
    char *temp = strtok(buff, splitters);
    P[0] = temp;
    int i = 1;
    while (temp != NULL)
    {
        temp = strtok(NULL, splitters);
        P[i] = temp;
        i++;
    }
    //if(new){free(new); new=NULL;}
    //if(d<ParaNum){P[d]=NULL;}
}

void StringStok_Gen(char *buff, char **P, char *g)
{
    //int d=Space_count1(buff);
    char *temp = strtok(buff, g);
    P[0] = temp;
    int i = 1;
    while (temp != NULL)
    {
        temp = strtok(NULL, g);
        P[i] = temp;
        i++;
    }
    //if(new){free(new); new=NULL;}
    //if(d<ParaNum){P[d]=NULL;}
}

void StringStokPipe(char *buff, char **P)
{
    //int d=Space_count1(buff);
    char *temp = strtok(buff, "|");
    P[0] = temp;
    int i = 1;
    while (temp != NULL)
    {
        temp = strtok(NULL, "|");
        P[i] = temp;
        i++;
    }
    //if(new){free(new); new=NULL;}
    //if(d<ParaNum){P[d]=NULL;}
}

// Parser1: Puts the replacers in place
void Parser_1(char *P)
{
    if (!P)
    {
        return;
    }
    int i = 0;
    while (P[i] != '\0')
    {
        if (P[i] == ' ' && P[i - 1] == '\\')
        {
            P[i] = replacer;
            P[i - 1] = replacer;
        }
        if (P[i] == '|' && P[i - 1] == '\\')
        {
            P[i] = PipRep;
            P[i - 1] = PipRep;
        }

        if (P[i] == '<' && P[i - 1] == '\\')
        {
            P[i] = InRep;
            P[i - 1] = InRep;
        }
        if (P[i] == '>' && P[i - 1] == '\\')
        {
            P[i] = OutRep;
            P[i - 1] = OutRep;
        }
        i++;
    }
}

// Removing 1 space
void SwapArr(char *P, int starter)
{
    if (!P)
    {
        return;
    }
    int i = starter + 1;
    while (P[i] != '\0')
    {
        P[i - 1] = P[i];
        i++;
    }
    P[i - 1] = '\0';
}

// Parser_2: Remove replacer and puts one space.
void Parser_2(char **args)
{
    if (args[0] == 0)
    {
        return;
    }
    int i = 0;
    while (args[i] != NULL)
    {
        int j = 0;
        while (args[i][j] != '\0')
        {
            if (args[i][j] == replacer && args[i][j - 1] == replacer)
            {
                args[i][j - 1] = ' ';
                SwapArr(args[i], j);
            }
            if (args[i][j] == PipRep && args[i][j - 1] == PipRep)
            {
                args[i][j - 1] = '|';
                SwapArr(args[i], j);
            }
            if (args[i][j] == InRep && args[i][j - 1] == InRep)
            {
                args[i][j - 1] = '<';
                SwapArr(args[i], j);
            }
            if (args[i][j] == OutRep && args[i][j - 1] == OutRep)
            {
                args[i][j - 1] = '>';
                SwapArr(args[i], j);
            }
            j++;
        }
        i++;
    }
}

// on empty string it returns -1,0.w 1
int HisCmd(char **args, char *p, int sp)
{
    if (args[0] == 0)
    {
        return -1;
    }
    strcpy(p, args[0]);
    int i = 1;
    while (args[i] != NULL)
    {
        if (sp)
        {
            strcat(p, " ");
        }
        strcat(p, args[i]);
        i++;
    }
    strcat(p, " \0");
    return 1;
}

// returns 1 => ls -al&,ls -al & -1 on empty string and 0 ow
int Checkamp(char **args)
{
    if (args[0] == 0)
    {
        return -1;
    }
    int i = 0;
    while (args[i] != NULL)
    {
        i++;
    }
    // last element of args[i-1] is &

    //printf("now\n");
    //printf("%d\n",args[i-1][strlen(args[i-1])-2]);
    //printf("%c\n",args[i-1][strlen(args[i-1])-2]);
    if (args[i - 1][strlen(args[i - 1]) - 1] == '&')
    {
        //printf("here\n");
        // current&& isn't acceptable
        if (strlen(args[i - 1]) > 1)
        {
            if (args[i - 1][strlen(args[i - 1]) - 2] == '&')
            {
                //printf("here (current&&)\n");
                return 0;
            }
        }
        // current& & isn't acceptable
        if (i > 1 && args[i - 2][strlen(args[i - 2]) - 1] == '&')
        {
            //printf("here (current& &)\n");
            return 0;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

// Enter duplicate string
char **Parse1st(char *p)
{
    if (!p)
    {
        return NULL;
    }
    Parser_1(p);
    int spca = Space_count_Pipe(p);
    char **args = (char **)malloc(spca * sizeof(char *));
    StringStokPipe(p, args);
    int i = 0;
    while (args[i] != NULL)
    {
        int space = Space_count1(args[i]);
        char **arg = (char **)malloc(space * sizeof(char *));
        StringStok(args[i], arg);

        char *p = (char *)malloc((strlen(args[i]) + 2) * sizeof(char));
        HisCmd(arg, p, 1);

        args[i] = p;
        if (arg)
        {
            free(arg);
            arg = NULL;
        }
        i++;
    }
    Parser_2(args);
    return args;
}

// Enter duplicate string if needed
char **ParserGen(char *String, char *spiltter)
{
    if (!String)
    {
        return NULL;
    }
    Parser_1(String);
    int Space = Space_count_Gen(String, spiltter);
    char **args = (char **)calloc(Space, sizeof(char *));
    StringStok_Gen(String, args, spiltter);
    Parser_2(args);
    return args;
}
