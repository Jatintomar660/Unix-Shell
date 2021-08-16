#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "parse.h"
#include "color.h"


char Name[500];

struct node
{
    unsigned int pid;
    char *Str;
    struct node *next;
};

struct node *History = NULL;
struct node *Current = NULL;
int ppid;

int InProcess;
int childPid;


void push(struct node **root, int pid_, char *p)
{
    struct node *Tmp = (struct node *)malloc(sizeof(struct node));
    Tmp->pid = pid_;
    Tmp->Str = p;
    Tmp->next = *root;
    *root = Tmp;
    //printf("%d Pushed\n", ind_);
}

int nodeDel(struct node **root, int pid_)
{
    if ((*root) == NULL)
    {
        return -1;
    }
    if ((*root)->pid == pid_)
    {
        struct node *temp = *root;
        *root = (*root)->next;
        if (temp)
        {
            free(temp);
            temp = NULL;
        }
        return 1; //head pointer is removed and next is new head
    }
    else
    {
        struct node *tmp = *root;
        while (((tmp->next) != NULL) && ((tmp->next)->pid != pid_))
        {
            tmp = tmp->next;
        }
        if (!(tmp->next))
        {
            return -2;
        } //NO such pid is present
        struct node *p = tmp->next;
        tmp->next = (tmp->next->next);
        if (p)
        {
            free(p);
            p = NULL;
            return 2;
        } //Pointer wasn't head pointer but deleted successfully
    }
}

int isEmpty(struct node *root)
{
    return !root;
}

// Signal Handler function
void handle_sigint(int sig)
{
    if (childPid != 0)
    {
        fprintf(stderr, "\n Process Killed \n");
        kill(childPid, SIGKILL);
    }
    //InProcess=1;nodeDel(&Current,pid);
    if (InProcess == 0)
    {
        fprintf(stderr, " To quit shell use STOP\n");
        fprintf(stderr, "%s", Name);
    }
}

// Alternative for child handler
void GiveStatus()
{
    pid_t pid;
    int status;
    if ((pid = waitpid(-1, &status, WNOHANG)) != -1)
    {
        //unregister_child(pid, status);   // Or whatever you need to do with the PID

        if (pid != 0)
        {
            fprintf(stderr, "Process [%d] ended with exit status: [%d]\n", pid, (status));
            nodeDel(&Current, pid);
        }
        // Process pid exited with exit status this.
        // remove pid from current list.
    }
}

// child handler function
void sigchld_handler(int signum)
{
    pid_t pid;
    int status;
    if ((pid = waitpid(-1, &status, WNOHANG)) != -1)
    {
        //unregister_child(pid, status);   // Or whatever you need to do with the PID

        if (pid != 0)
        {
            fprintf(stderr, "Process [%d] ended with exit status: [%d]\n", pid, (status));
            nodeDel(&Current, pid);
        }
        // Process pid exited with exit status this.
        // remove pid from current list.
    }
    //fprintf(stderr,"%s",Name);
}

// List Display for debugging purposes
void DisplayList(struct node *root)
{
    while (root != NULL)
    {
        fprintf(stderr, " %d # %s | ", root->pid, root->Str);
        root = root->next;
    }
    fprintf(stderr, "\n");
}

// Display function for HISTn
void DisplayHistn(struct node *root, int n)
{
    fprintf(stderr, "\n");
    if (!root)
    {
        fprintf(stderr, "Nothing to Display \n");
        return;
    }
    int i = 0;
    while (root != NULL && (i != n))
    {
        i++;
        char *new;
        new = strdup(root->Str);
        int j = 0;
        while (new[j] != '\0' && (new[j] != ' '))
        {
            j++;
        }
        new[j] = '\0';

        fprintf(stderr, "%d. %s\n", i, root->Str);
        root = root->next;

        if (new)
        {
            free(new);
            new = NULL;
        }
    }
    fprintf(stderr, "\n");
}

// Prints all process name with pid
void Disp_Pid(struct node *root)
{
    if (!root)
    {
        return;
    }
    int i = 0;
    while (root != NULL)
    {
        i++;
        int j = 0;
        char *new;
        new = strdup(root->Str);
        while (new[j] != '\0' && (new[j] != ' '))
        {
            j++;
        }
        new[j] = '\0';

        fprintf(stderr, "Command name: %s     Process id: %d\n", new, root->pid);
        root = root->next;

        // freeing the memory
        if (new)
        {
            free(new);
            new = NULL;
        }
    }
    fprintf(stderr, "\n");
}

// Returns total elements in linked list
int TotalElement(struct node *root)
{
    if (!root)
    {
        return 0;
    }
    int i = 0;
    while (root != NULL)
    {
        root = root->next;
        i++;
    }
    return i;
}

// Returned a duplicate string needs to be freed
char *ExecHistnStr(struct node *root, int n)
{
    if (!root)
    {
        return "";
    }
    int i = TotalElement(root);
    int j = 0;
    while (root != NULL && j != i - n)
    {
        root = root->next;
        j++;
    }
    if (root == NULL)
    {
        return "";
    }

    char *p = strdup(root->Str);
    return p;
}


void ArgDisp(char **Args)
{
    if (Args[0] == 0)
    {
        //printf("input is NULL\n");
        return;
    }
    int i = 0;
    while (Args[i] != NULL)
    {
        printf("%d. %s  ", i + 1, Args[i]);
        i++;
    }
    printf("%d. NULL \n", i + 1);
}

// check cd command -1 on empty string, 1 on correct cmd and 0 o.w .
int Checkcd(char **args)
{
    if (args[0] == 0)
    {
        return -1;
    }
    if (strcmp(args[0], "cd") == 0)
    {
        int i = 0;
        while (args[i] != NULL)
        {
            i++;
        }
        if (i > 2)
        {
            return 0;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

// check pid command (pid, 1)(pid current, 2) (pid all, 3) (not found,0) (empty string,-1)
int Checkpid(char **args)
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
    if (i == 1 && strcmp(args[0], "pid") == 0)
    {
        return 1;
    }
    if (i == 2 && strcmp(args[0], "pid") == 0 && strcmp(args[1], "current") == 0)
    {
        return 2;
    }
    if (i == 2 && strcmp(args[0], "pid") == 0 && strcmp(args[1], "all") == 0)
    {
        return 3;
    }
    else
    {
        return 0;
    }
}

// (num is 120=> cwd)  (min num 60=>hostname)  and  (somin is 16=>username)
void Shellname()
{
    char s[num];
    char hname[minNum];
    char nae[somin];
    getlogin_r(nae, somin);
    gethostname(hname, minNum);
    getcwd(s, num);
    strcpy(Name, "<");
    strcat(Name, nae);
    strcat(Name, "@");
    strcat(Name, hname);
    strcat(Name, ":~");
    strcat(Name, s);
    strcat(Name, "> ");
}

// return -1 for null args and -2 when string is not HISTN, N otherwise
int CheckHistn(char **args)
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
    if (i == 1 && strncmp(args[0], "HIST", 4) == 0)
    {
        if (strlen(args[0]) > 4)
        {
            int n;
            sscanf(&args[0][4], "%d", &n);
            return n;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -2;
    }
}

// return -1 for null args and -2 when string is not !HISTN, N otherwise
int CExecHistn(char **args)
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
    if (i == 1 && strncmp(args[0], "!HIST", 4) == 0)
    {
        if (strlen(args[0]) > 5)
        {
            int n;
            sscanf(&args[0][5], "%d", &n);
            return n;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -2;
    }
}

// Checks whether kill command is given or not
int CheckKIll(char **args)
{
    if (args[0] == 0)
    {
        return -1;
    }
    if (strcmp(args[0], "kill") == 0)
    {
        int n;
        sscanf(args[1], "%d", &n);
        return n;
    }
    else
    {
        return 0;
    }
}

// Kills process given pid of that process
void PKill(int n)
{
    int m = kill(n, SIGKILL);
    //kill(n,SIGKILL);
    nodeDel(&Current, n);
    if (m == -1)
    {
        //fprintf(stderr,"Error in ending the Process [%d] \n",n);
        return;
    }
    fprintf(stderr, "Process [%d] Ended Sucessfully \n", n);
    return;
}

// Help Message
void PrintMessage()
{

    fprintf(stderr, "------------------------------------\n");
    fprintf(stderr, "Usage of Shell\n");
    fprintf(stderr, "STOP                       : Stop executing shell \n");
    fprintf(stderr, "HISTN                      : Returns last N (int) commands \n");
    fprintf(stderr, "!HISTN                     : Execute Nth (int) command (from start) \n");
    fprintf(stderr, "cd                         : Command used to change directory \n");
    fprintf(stderr, "pid                        : Returns process id of shell \n");
    fprintf(stderr, "pid current                : Returns process id of currently active processes \n");
    fprintf(stderr, "pid all                    : Returns process id of all processes spawned by shell \n");
    fprintf(stderr, "Help                       : Returns usage information of shell (additional command format) \n");
    fprintf(stderr, "Note: For spaces in input use <backslash><space> like UNIX systems \n");
    fprintf(stderr, "------------------------------------\n");
    fprintf(stderr, "\n");
}

void Interaction()
{

    fprintf(stderr, "------------------------------------\n");
    fprintf(stderr, "LIST OF ADDITIONAL COMMANDS FOR USER\n");
    fprintf(stderr, "STOP                       : Stop executing shell \n");
    fprintf(stderr, "HISTN                      : Returns last N (int) commands \n");
    fprintf(stderr, "!HISTN                     : Execute Nth (int) command (from start) \n");
    fprintf(stderr, "cd                         : Command used to change directory \n");
    fprintf(stderr, "pid                        : Returns process id of shell \n");
    fprintf(stderr, "pid current                : Returns process id of currently active processes \n");
    fprintf(stderr, "pid all                    : Returns process id of all processes spawned by shell \n");
    fprintf(stderr, "Help                       : Returns usage information of shell (additional command format) \n");
    fprintf(stderr, "------------------------------------\n");
    fprintf(stderr, "\n");
}

// Checks Help in input string
int CheckHelp(char **args)
{
    if (args[0] == 0)
    {
        return -1;
    }
    if (strcmp(args[0], "Help") == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// Execute foreground processes and returns child pid
// args: array of strings, p: string to be saved,
//f: whether given input needs to be saved or not (f=1 save f=0 don't save).
int Execute(char **args, char *p, int f)
{
    InProcess = 1;
    if (args[0] == 0)
    {
        return -1;
    }
    int rd = fork();
    if (rd == -1)
    {
        fprintf(stderr, "Error in Forkinig\n");
        return -2;
    }
    if (rd == 0)
    {
        childPid = getpid();
        //int n=getpid();
        //fprintf(stderr,"\n");
        execvp(args[0], args);
        fprintf(stderr, "Command not Found (type 'Help' if required) \n");
        exit(0);
    }
    else
    {
        // putting in history
        if (f == 1)
        {
            push(&History, rd, p);
        }
        int status;
        waitpid(rd, &status, 0);
        //fprintf(stderr,"\n");
        childPid = 0;
        //fprintf(stderr,"Process [%d] is ended \n",rd);
    }
    return rd;
}

// Execute background processes and returns the pid of child process
// args: array of strings, p: string to be saved,
//f: whether given input needs to be saved or not. (f=1 save f=0 don't save)
int ExecAmp(char **args, char *p, int f)
{
    InProcess = 1;
    if (args[0] == 0)
    {
        return -1;
    }
    int rd = fork();
    if (rd == -1)
    {
        fprintf(stderr, "Error in Forkinig\n");
        return -2;
    }
    if (rd == 0)
    {
        int n = getpid();
        fprintf(stderr, "\nProcess [%d] is started in background\n", n);
        setpgid(0, 0);
        execvp(args[0], args);
        fprintf(stderr, "Command not Found (type 'Help' if required) \n");
        exit(0);
    }
    else
    {
        // Putting in history
        if (f == 1)
        {
            push(&History, rd, p);
        }
        push(&Current, rd, p);
        fprintf(stderr, "\n");
        // int status;
        // waitpid(-1,&status,WNOHANG);
    }
    return rd;
}

// function to kill all processes
void KillAll()
{
    if (!Current)
    {
        return;
    }
    else
    {
        while (Current != NULL)
        {
            PKill(Current->pid);
            //nodeDel(&Current,Current->pid);
        }
    }
}

// Remove & from strings
void AmpInp(char **args)
{
    if (args[0] == 0)
    {
        return;
    }
    int i = 0;
    while (args[i] != NULL)
    {
        i++;
    }

    if (strcmp(args[i - 1], "&") == 0)
    {
        args[i - 1] = NULL;
    }
    else
    {
        args[i - 1][strlen(args[i - 1]) - 1] = '\0';
    }
}

// Check STOP in array of strings
int StopI(char **args)
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
    if (strcmp(args[0], "STOP") == 0 && i == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//(Return 1 for <) (Return 2 for >) (Return 3 for > <) (Return 4  for |)
// (Return 5 for < |) (Return 6 for > |)(Return 7 for | > <) (Return 0 (for not pipe but fg,bg is possible))
// (Return -1 for empty string) // (Return -2 for syntax error)
int CheckFileIP(char *p)
{
    if (!p)
    {
        return -1;
    }
    int i = 0;
    int P = 0;
    int I = 0;
    int O = 0;
    while (p[i] != '\0')
    {
        if (p[i] == '|' && p[i - 1] != '\\')
        {
            P = 1;
        }
        if (p[i] == '>' && p[i - 1] != '\\')
        {
            O = 1;
        }
        if (p[i] == '<' && p[i - 1] != '\\')
        {
            I = 1;
        }
        i++;
    }
    int a = 0;
    int b = 0;
    int c = 0;
    int n = 0;
    i = 0;
    while (p[i] != '\0')
    {
        n = p[i];
        if (p[i] == '<' && p[i - 1] != '\\' || p[i] == '>' && p[i - 1] != '\\' || p[i] == '&' && p[i - 1] != '\\')
        {
            if (c == 1)
            {
                b = 1;
                break;
            }
            a = 1;
        }
        if (n >= 65 && n <= 90 || n >= 97 && n <= 122)
        {
            a = 0;
            c = 0;
        }
        if (p[i] == ' ' || p[i] == '\t')
        {
            i++;
            continue;
        }
        if (p[i] == '|')
        {
            c = 1; // c==1 implies that pipe is reached
            if (a == 1)
            {
                b = 1; // b==1 implies syntax error (for breaking the loop)
                break;
            }
        }
        i++;
    }
    if (P == 1 && b == 1)
    {
        return -2;
    }
    if (P && O && I)
    {
        return 7;
    }
    else if (P && O && !I)
    {
        return 6;
    }
    else if (P && !O && I)
    {
        return 5;
    }
    else if (!P && O && I)
    {
        return 3;
    }
    else if (!P && O && !I)
    {
        return 2;
    }
    else if (P && !O && !I)
    {
        return 4;
    }
    else if (!P && !O && I)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void FreeMem(char **args)
{
    if (!args)
    {
        return;
    }
    if (args)
    {
        free(args);
        args = NULL;
    }
}

// return 1 on normal execution
// return 2 on background
// return 3 on pipe
// return 4 on pipe and background
// return -2 on syntax error at pipe
// return -1 O.W
int Distributor(char **args, char *Sline)
{
    int P = CheckFileIP(Sline);
    int Amp = Checkamp(args);

    if (P == -2)
    {
        return -2;
    }

    if (P > 0 && Amp)
    {
        return 4;
    }
    else if (P > 0 && Amp < 1)
    {
        return 3;
    }
    else if (P == 0 && Amp)
    {
        return 2;
    }
    else if (P == 0 && Amp < 1)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

// args: Array of command and parameter, NameFile: Name of Inp/Out file
// Back: flag (1) for background process
// wc < hello , wc hello > hello1
// eg. args={"wc",NULL}, NameFile="hello", Inp=1
// eg. args={"wc","hello",NULL}, NameFile="hello1", InOut=0
int IORedirect(char **args, char *NameFile, int Inp, int Back, char *StrHis)
{
    int rd = fork();
    if (rd == -1)
    {
        fprintf(stderr, "Error in fork\n");
    }
    if (rd == 0)
    {

        if (NameFile != NULL)
        {
            if (Inp)
            {
                int File = open(NameFile, O_RDONLY, 0777);
                if (File == -1)
                {
                    fprintf(stderr, "File Does not exist\n");
                    exit(0);
                    return 4;
                }
                dup2(File, STDIN_FILENO);
                close(File);
            }
            else
            {
                int File = open(NameFile, O_WRONLY | O_CREAT, 0777);
                if (File == -1)
                {
                    fprintf(stderr, "File Does not exist\n");
                    exit(0);
                    return 4;
                }
                dup2(File, STDOUT_FILENO);
                close(File);
            }
        }
        int n = getpid();
        if (Back)
        {
            setpgid(0, 0);
            push(&Current, n, StrHis);
        }
        execvp(args[0], args);
        fprintf(stderr, "Command not Found !!\n");
        exit(0);
    }

    // Saving in History
    push(&History, rd, StrHis);

    if (!Back)
    {
        wait(NULL);
    }

    return rd;
}

// args: Array of command and parameter, NameFI: Name of Inpt file
// NameFO: Name of output file , Back: flag (1) for background process
// wc < hello > hello1
// eg. args={"wc",NULL}, NameFI="hello",  NameFO="hello1"
int IORedirectBth(char **args, char *NameFI, char *NameFO, int Back, char *StrHis)
{
    int rd = fork();
    if (rd == -1)
    {
        fprintf(stderr, "Error in fork\n");
    }
    if (rd == 0)
    {
        int n = getpid();

        if (NameFI != NULL)
        {
            int File1 = open(NameFI, O_RDONLY, 0777);
            if (File1 == -1)
            {
                fprintf(stderr, "Input file Does not exist\n");
                exit(0);
                return 4;
            }
            dup2(File1, STDIN_FILENO);
            close(File1);
        }

        if (NameFO != NULL)
        {
            int File2 = open(NameFO, O_WRONLY | O_CREAT, 0777);
            if (File2 == -1)
            {
                fprintf(stderr, "Output file Does not exist\n");
                exit(0);
                return 4;
            }
            dup2(File2, STDOUT_FILENO);
            close(File2);
        }

        if (Back)
        {
            setpgid(0, 0);
            push(&Current, n, StrHis);
        }
        execvp(args[0], args);
        fprintf(stderr, "Command not Found !!\n");
        exit(0);
    }

    // Saving HIstory
    push(&History, rd, StrHis);
    if (!Back)
    {
        wait(NULL);
    }

    return rd;
}

// arg: array of string of arrays, Numprocess: size of arg
// ping -c 5 google.com | grep rtt
// char *args1[]={"ping","-c","5","google.com",NULL};
// char *args2[]={"grep","rtt",NULL};
// char **arg[]={args1,args2};
// Numprocess=2
int PipeExecution(char ***arg, int NumProcess, int Back, char **StrHis)
{
    // Creating file Descriptor
    int **fd = (int **)malloc(NumProcess * sizeof(int *));
    for (int i = 0; i < NumProcess; i++)
    {
        fd[i] = (int *)malloc(sizeof(int *));
    }
    // Creating Pipes
    for (int i = 0; i < NumProcess; i++)
    {
        if (pipe(fd[i]) == -1)
        {
            fprintf(stderr, "Error Occured in Piping\n");
            return -1;
        }
    }

    // Creating Processes
    for (int i = 0; i < NumProcess; i++)
    {
        int rd = fork();
        if (rd == -1)
        {
            fprintf(stderr, "Error Occured in Forking\n");
            return -2;
        }

        // Saving Command in History pid only
        push(&History, rd, StrHis[i]);

        if (rd == 0)
        {
            int n = getpid();

            // Closing File descriptors
            for (int j = 0; j < NumProcess; j++)
            {
                // close read end
                if (j != i - 1)
                {
                    close(fd[j][0]);
                }

                // close write end
                if (j != i)
                {
                    close(fd[j][1]);
                }
            }

            // Taking fd[i-1] as stdin (only first process isn't reading)
            if (i != 0)
            {
                dup2(fd[i - 1][0], STDIN_FILENO);
                close(fd[i - 1][0]); // closing original fd's
            }
            // Everyone is writing in stdout except last one
            if (i != NumProcess - 1)
            {
                dup2(fd[i][1], STDOUT_FILENO);
                close(fd[i][1]); // closing original fd's
            }

            if (Back)
            {
                setpgid(0, 0);
                push(&Current, n, StrHis[i]);
            }
            execvp(arg[i][0], arg[i]);
            fprintf(stderr, "Command not Found!!\n");
            exit(0);
        }
    }

    // Main will not use any fd
    // Closing File descriptors
    for (int j = 0; j < NumProcess; j++)
    {
        // close read and write ends of all pipes in main
        close(fd[j][0]);
        close(fd[j][1]);
    }

    if (!Back)
    {
        for (int i = 0; i < NumProcess; i++)
        {
            wait(NULL);
        }
    }

    return 1;
}

// arg: array of string of arrays, Numprocess: size of arg
// NameFI: array of string (Contain file names with input redirection if exist, O.W NULL)
// NameFO: array of string (Contain file names with input redirection if exist, O.W NULL)
// InOut: 0 Output Redirection, 1 Input redirection, 2 Both Input Output Redirection
// StrHis: String to be saved in History
// Eg. cat < hello | wc, cat hello | wc > output.txt, cat < hello | wc > Output.txt
// char *args1[]={"cat",NULL}; , char *args2[]={"wc",NULL}; %% char **arg[]={args1,args2}; %% Numprocess=2
// NameFI:  1)  {hello,NULL},  2) NULL ,              3) {hello,NULL}
// NameFI:  1)  NULL,          2) {output.txt,NULL} , 3) {Output.txt,NULL}
// InOut :  1) 1               2) 0                   3) 2
int PipeAllExec(char ***arg, char **NameFI, char **NameFO, int NumProcess, int InOut, int Back, char **StrHis)
{
    // Creating file Descriptor
    int **fd = (int **)malloc(NumProcess * sizeof(int *));
    for (int i = 0; i < NumProcess; i++)
    {
        fd[i] = (int *)malloc(sizeof(int *));
    }

    // Creating Pipes
    for (int i = 0; i < NumProcess; i++)
    {
        if (pipe(fd[i]) == -1)
        {
            fprintf(stderr, "Error Occured in Piping\n");
            return -1;
        }
    }

    // Creating Processes
    for (int i = 0; i < NumProcess; i++)
    {
        int rd = fork();
        if (rd == -1)
        {
            fprintf(stderr, "Error Occured in Forking\n");
            return -2;
        }

        // Saving Command in History pid only
        push(&History, rd, StrHis[i]);

        if (rd == 0)
        {
            // Saving History
            int n = getpid();

            // Closing File descriptors
            for (int j = 0; j < NumProcess; j++)
            {
                // close read end
                if (j != i - 1)
                {
                    close(fd[j][0]);
                }

                // close write end
                if (j != i)
                {
                    close(fd[j][1]);
                }
            }
            // Taking fd[i-1] as stdin (only first process isn't reading)
            if (i != 0)
            {
                dup2(fd[i - 1][0], STDIN_FILENO);
                close(fd[i - 1][0]); // closing original fd's
            }
            // Everyone is writing in stdout except last one
            if (i != NumProcess - 1)
            {
                dup2(fd[i][1], STDOUT_FILENO);
                close(fd[i][1]); // closing original fd's
            }
            // Input Redirection

            if (NameFI != NULL && (InOut == 1 || InOut == 2))
            {
                if (NameFI[i] != NULL && strlen(NameFI[i]) > 0)
                {
                    // Assigning Reading part to fd[0][0] (if Input redirection is done)
                    fd[i][0] = open(NameFI[i], O_RDONLY, 0777);
                    dup2(fd[i][0], STDIN_FILENO);
                    close(fd[i][0]);
                }
            }
            if (NameFO != NULL && (InOut == 0 || InOut == 2))
            {
                if (NameFO[i] != NULL && strlen(NameFO[i]) > 0)
                {
                    // Assigning Writing part to fd[NumProcess-1][1] (if output redirection is done)
                    fd[i][1] = open(NameFO[i], O_WRONLY | O_CREAT, 0777);
                    dup2(fd[i][1], STDOUT_FILENO);
                    close(fd[i][1]);
                }
            }
            if (Back)
            {
                setpgid(0, 0);
                push(&Current, n, StrHis[i]);
            }
            execvp(arg[i][0], arg[i]);
            fprintf(stderr, "Command not Found!!\n");
            exit(0);
        }
    }

    // Main will not use any fd
    // Closing File descriptors
    for (int j = 0; j < NumProcess; j++)
    {
        // close read and write ends of all pipes in main
        close(fd[j][0]);
        close(fd[j][1]);
    }

    if (!Back)
    {
        for (int i = 0; i < NumProcess; i++)
        {
            wait(NULL);
        }
    }

    return 1;
}

// Remove extra spaces from string
char *Trim(char *buff)
{
    if (!buff)
    {
        return NULL;
    }
    char *new = (char *)malloc((strlen(buff) + 1) * sizeof(char));
    int i = 0;
    int c = 0;
    while (buff[i] != '\0')
    {
        int t = buff[i];
        if (t == 32 || t == 10 || t == 9)
        {
            i++;
            continue;
        }
        else
        {
            //printf("buff[%d]: %c ,Ascii: %d %d %d\n",i,buff[i],buff[i],'\n','\t');
            new[c] = buff[i];
            i++;
            c++;
        }
    }
    new[c] = '\0';
    return new;
}

int PIPE_IO_ALL(char *Sline)
{
    char *stringdup = strdup(Sline);
    int spc = Space_count1(stringdup);
    char **args = Parse1st(stringdup);
    //ArgDisp(args);

    // History string
    char *StrHis = (char *)malloc((strlen(Sline) + 1) * sizeof(char));
    HisCmd(args, StrHis, 1);

    // Only Pipe
    int TypeCheck = CheckFileIP(Sline);

    if (TypeCheck >= 1 && TypeCheck <= 3)
    {

        char *NameFI = NULL;
        char *NameFO = NULL;

        char *new = strdup(args[0]);
        int Sze = Space_count_Gen(new, ">");
        char **Avg = ParserGen(new, ">");
        //ArgDisp(Avg);

        if (Sze > 2)
        {
            NameFO = Trim(Avg[Sze - 2]);
        }

        //fprintf(stderr,"NameFO : %s\n",NameFO);

        char *new2 = strdup(Avg[0]);
        int Sze2 = Space_count_Gen(new2, "<");
        char **Avg2 = ParserGen(new2, "<");
        //ArgDisp(Avg2);

        if (Sze2 > 2)
        {
            NameFI = Trim(Avg2[Sze2 - 2]);
        }

        char *new3 = strdup(Avg2[0]);
        char **Avg3 = ParserGen(new3, splitters);

        //ArgDisp(Avg3);
        //printf("Inp: %s, Out : %s \n",NameFI,NameFO);

        // Input Redirection
        if (TypeCheck == 1)
        {
            IORedirect(Avg3, NameFI, 1, 0, StrHis);
            // if(Avg3){free(Avg3);Avg3=NULL;}
            // if(Avg2){free(Avg2);Avg2=NULL;}
            // if(Avg){free(Avg);Avg=NULL;}
            // if(new3){free(new3);new3=NULL;}
            // if(new2){free(new2);new2=NULL;}
            // if(new){free(new);new=NULL;}

            // if(args){free(args);args=NULL;}
        }
        // Output Redirection
        else if (TypeCheck == 2)
        {
            IORedirect(Avg3, NameFO, 0, 0, StrHis);
            // if(Avg3){free(Avg3);Avg3=NULL;}
            // if(Avg2){free(Avg2);Avg2=NULL;}
            // if(Avg){free(Avg);Avg=NULL;}
            // if(new3){free(new3);new3=NULL;}
            // if(new2){free(new2);new2=NULL;}
            // if(new){free(new);new=NULL;}

            // if(args){free(args);args=NULL;}
        }
        // In Out Both
        else
        {
            IORedirectBth(Avg3, NameFI, NameFO, 0, StrHis);
            // if(Avg3){free(Avg3);Avg3=NULL;}
            // if(Avg2){free(Avg2);Avg2=NULL;}
            // if(Avg){free(Avg);Avg=NULL;}
            // if(new3){free(new3);new3=NULL;}
            // if(new2){free(new2);new2=NULL;}
            // if(new){free(new);new=NULL;}

            // if(args){free(args);args=NULL;}
        }
    }
    else if (TypeCheck == 4)
    {
        // Only Pipe (args is saving in history should not be freed)
        //ArgDisp(args);
        int NumProc = 0;
        while (args[NumProc] != NULL)
        {
            NumProc++;
        }
        //printf("%d\n",NumProc);
        char ***arg = (char ***)malloc((NumProc + 1) * sizeof(char **));

        int i = 0;
        while (args[i] != NULL)
        {
            char *New = strdup(args[i]);
            arg[i] = ParserGen(New, splitters);
            //printf("--------ArgDisp---i: %d------\n",i);
            //ArgDisp(arg[i]);
            i++;
        }

        PipeExecution(arg, NumProc, 0, args);
    }
    else if (TypeCheck >= 5 && TypeCheck <= 7)
    {
        //Pipe with In out
        int NumProc = 0;
        while (args[NumProc] != NULL)
        {
            NumProc++;
        }
        //printf("%d\n",NumProc);

        char **NameFI = (char **)malloc((NumProc + 1) * sizeof(char *));
        char **NameFO = (char **)malloc((NumProc + 1) * sizeof(char *));
        char ***arg = (char ***)malloc((NumProc + 1) * sizeof(char **));

        for (int i = 0; i < NumProc; i++)
        {
            char *new = strdup(args[i]);
            int Sze = Space_count_Gen(new, ">");
            char **Avg = ParserGen(new, ">");
            //ArgDisp(Avg);

            if (Sze > 2)
            {
                NameFO[i] = Trim(Avg[Sze - 2]);
            }

            //fprintf(stderr,"NameFO : |%s|\n",NameFO[i]);

            char *new2 = strdup(Avg[0]);
            int Sze2 = Space_count_Gen(new2, "<");
            char **Avg2 = ParserGen(new2, "<");
            //ArgDisp(Avg2);

            if (Sze2 > 2)
            {
                NameFI[i] = Trim(Avg2[Sze2 - 2]);
            }
            //fprintf(stderr,"NameFI : |%s|\n",NameFI[i]);

            char *new3 = strdup(Avg2[0]);
            char **Avg3 = ParserGen(new3, splitters);
            arg[i] = Avg3;
            //ArgDisp(arg[i]);
        }
        if (TypeCheck == 5)
        {
            // Pipe with <
            PipeAllExec(arg, NameFI, NameFO, NumProc, 1, 0, args);
        }
        else if (TypeCheck == 6)
        {
            // Pipe with >
            PipeAllExec(arg, NameFI, NameFO, NumProc, 0, 0, args);
        }
        else
        {
            // Pipe with > <
            PipeAllExec(arg, NameFI, NameFO, NumProc, 2, 0, args);
        }
    }
    else
    {
        // Not executable from pipe (Probably syntax error or invalid input)
        fprintf(stderr, "Invalid Input (in Pipe) \n");
    }
}

// fg=1 for bg 0 for bg
void FG_BG_Execution(char **args, char *Sline, int Fg)
{

    if (!args)
    {
        return;
    }
    // Check whether cd command is given or not
    if (Checkcd(args) > 0)
    {

        char *save = strdup(Sline);
        if (args[1] != 0)
        {
            chdir(args[1]);
        }
        Shellname();
        push(&History, ppid, save);
    }
    // Checking whether command is of pid format or not
    else if (Checkpid(args) > 0)
    {
        int cpid = Checkpid(args);
        if (cpid == 1)
        {
            fprintf(stderr, "\nCommand name: %s    Process id: %d\n", executable, ppid);
        }
        else if (cpid == 2)
        {
            //Current

            if (!Current)
            {
                fprintf(stderr, "\nThere are no currently active processes\n");
            }
            else
            {
                fprintf(stderr, "\nList of currently executing processes spawned from this shell: \n");
                Disp_Pid(Current);
            }
        }
        else
        {
            //All (cpid==3)

            if (!History)
            {
                fprintf(stderr, "\nProcesses aren't spawned from this shell yet! \n");
            }
            else
            {
                fprintf(stderr, "\nList of all processes spawned from this shell: \n");
                Disp_Pid(History);
            }
        }
    }
    // Checking whether the command is HISTn or not
    else if (CheckHistn(args) >= 0)
    {
        int n = CheckHistn(args);
        if (!History)
        {
            fprintf(stderr, "\nNothing to Display\n");
        }
        else if (n == 0)
        {
            fprintf(stderr, "\nPlease enter some number (>0) immediately after HIST \n");
        }
        else
        {
            DisplayHistn(History, n);
        }
    }
    // Checking whether the command is !HISTn or not
    else if (CExecHistn(args) >= 0)
    {
        int n = CExecHistn(args);
        if (!History)
        {
            fprintf(stderr, "\nNothing to Execute\n");
        }
        else if (n == 0)
        {
            fprintf(stderr, "\nPlease enter some number (>0) immediately after !HIST \n");
        }
        else
        {
            int n = CExecHistn(args);
            char *H = ExecHistnStr(History, n);
            size_t Hlen = strlen(H);
            if (Hlen > 0)
            {
                int spac = Space_count1(H);
                char *new = strdup(H);
                int FileIP = CheckFileIP(new);
                Parser_1(H);
                char **argsl = (char **)malloc(spac * sizeof(char *));
                StringStok(H, argsl);
                Parser_2(argsl);

                if (Checkcd(argsl) > 0)
                {
                    if (argsl[1] != 0)
                    {
                        chdir(argsl[1]);
                    }
                    Shellname();
                }
                else if (FileIP > 0)
                {
                    PIPE_IO_ALL(new);
                }
                else
                {
                    char *p = (char *)malloc((Hlen + 2) * sizeof(char));
                    HisCmd(argsl, p, 1);
                    // Execute
                    Execute(argsl, p, 0);
                    InProcess = 0;
                    if (p)
                    {
                        free(p);
                        p = NULL;
                    }
                }
                if (H)
                {
                    free(H);
                    H = NULL;
                }
                if (argsl)
                {
                    free(argsl);
                    argsl = NULL;
                }
            }
            else
            {
                fprintf(stderr, "Can't reach for command number (%d) \n", n);
            }
        }
    }
    // Checking whether the command is Kill or not
    else if (CheckKIll(args) > 0)
    {
        int n = CheckKIll(args);
        PKill(n);
    }
    // Checking whether the command is Help or not
    else if (CheckHelp(args) > 0)
    {
        PrintMessage();
    }
    // Executing and saving the given command
    else
    {
        char *p = (char *)malloc((strlen(Sline) + 2) * sizeof(char));
        HisCmd(args, p, 1);
        // Execute.
        if (Fg)
        {
            Execute(args, p, 1);
        }
        else
        {
            ExecAmp(args, p, 1);
        }
        InProcess = 0;
    }
    fprintf(stderr, "%s", Name);
}

int main()
{

    White();
    ppid = getpid();
    signal(SIGINT, handle_sigint);
    //signal(SIGCHLD,sigchld_handler);

    char *Sline = NULL;
    size_t Slen = 0;
    ssize_t Sread;

    Interaction();
    Shellname();

    //fflush(stdin);

    //Name of shell
    fprintf(stderr, "%s", Name);
    int i = 0;

    //char *S=NULL;
    while ((Sread = getline(&Sline, &Slen, stdin)) != -1)
    {

        Sline[strlen(Sline) - 1] = '\0';
        //fprintf(stderr,"Sline ;  %s\n",Sline);

        char *S = strdup(Sline);
        char **Arg2 = ParserGen(S, splitters);
        //ArgDisp(Arg2);

        int Distrib = Distributor(Arg2, Sline);

        GiveStatus();

        if (strcmp(Sline, "STOP") == 0)
        {
            KillAll();
            fprintf(stderr, "Closing the shell\n");
            break;
        }
        if (Distrib == 4)
        {
            // Pipe and ampersant
            char *New = strdup(Sline);
            char *temp = NULL;
            temp = strtok(New, "&");
            char *NewSline = strdup(temp);
            PIPE_IO_ALL(NewSline);
            if (NewSline != NULL)
            {
                free(NewSline);
                NewSline = NULL;
            }
            if (New != NULL)
            {
                free(New);
                New = NULL;
            }

            fprintf(stderr, "%s ", Name);
        }
        else if (Distrib == 3)
        {
            // Freeing Arg2
            if (Arg2 != NULL)
            {
                free(Arg2);
                Arg2 = NULL;
            }
            PIPE_IO_ALL(Sline);
            fprintf(stderr, "%s ", Name);
        }
        else if (Distrib == 2)
        {
            // Background Process
            AmpInp(Arg2);
            if (StopI(Arg2) > 0)
            {
                KillAll();
                fprintf(stderr, "Closing the shell\n");
                break;
            }
            FG_BG_Execution(Arg2, Sline, 0);
        }
        else if (Distrib == 1)
        {
            // Foreground Process
            if (StopI(Arg2) > 0)
            {
                KillAll();
                fprintf(stderr, "Closing the shell\n");
                break;
            }
            FG_BG_Execution(Arg2, Sline, 1);
        }
        else if (Distrib == -2)
        {
            // Syntax Error;
            fprintf(stderr, "Syntax Error Near '|' \n");
        }
        else
        {
            // Invalid Input
            fprintf(stderr, "Invalid Input \n");
        }
    }
    KillAll();
    if (Sline)
    {
        free(Sline);
        Sline = NULL;
    }

    return 0;
}
