#include<stdio.h>
#include "shell.h"
char prompt_str[] = "minishell$:";
char input_str[25];
int main()
{
    system("clear");//to clear the previous cmds or prompt to print our string in the top
    
    scan_input(prompt_str,input_str);
}