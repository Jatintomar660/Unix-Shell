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
// Color Codes
#define Magenta "\033[34m"
#define Red "\033[31m"
#define Green "\033[32m"
#define Yellow "\033[33m"
#define Cyan "\033[36m"

#define BMagenta "\033[1;34m"
#define BBlack "\033[1;30m"
#define BRed "\033[1;31m"
#define BGreen "\033[1;32m"
#define BYellow "\033[1;33m"
#define BCyan "\033[1;36m"

#define CDefualt "\033[0m"
#define Bold "\033[1;39m"

void White() { printf(Bold); }
void BRed_() { printf(BRed); }
void Red_() { printf(Red); }
void BBlack_() { printf(BBlack); }
void Green_() { printf(Green); }
void BGreen_() { printf(BGreen); }
void Yellow_() { printf(Yellow); }
void BYellow_() { printf(BYellow); }
void Magenta_() { printf(Magenta); }
void BMagenta_() { printf(BMagenta); }
void Cyan_() { printf(Cyan); }
void BCyan_() { printf(BCyan); }
void CReset() { printf(CDefualt); }