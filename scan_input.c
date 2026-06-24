#include "shell.h"
char *external_commands[154];
extern char prompt_str[];
int pid=0;
int status;
extern char input_str[25];
Slist *head=NULL;
int insert_at_first(Slist **head, int pid,char *command)
{
    Slist *new=malloc(sizeof(Slist));//creating new node
    if(new==NULL) //checking new node created or not
    return -1;
    //updating new data and link
    new->pid=pid;
    strcpy(new->command,command);
    new->link=*head;
    *head=new;
	return 1;
}
 void signal_handler(int signum)
 {
    if(signum==SIGINT)
    {
        if(pid==0)//only if user pressing cntrl c or cntrl z along with anything is passed no need to print
        {
             //print the prompt..bcz every time when we press cntrl+c it will print prompt
        //printf("%s",prompt_str);//minishell$
        fflush(stdout);
        }
       
    }
    else if(signum==SIGTSTP)
    {
        if(pid==0)
        {
             //printf("%s",prompt_str);
             fflush(stdout);
        }
        // else
        // {
        //     //insert first logic =>for storing stopped process in linked list
        //     insert_at_first(&head,pid,input_str);
        // }
        
    }
    else if(signum==SIGCHLD)
    {
        //how to clear the resource
        while(waitpid(-1, &status, WNOHANG) > 0);
    }
 }

void scan_input(char *prompt_str,char *input_str)
{
    extract_external_commands(external_commands);

    signal(SIGINT,signal_handler);
    signal(SIGTSTP,signal_handler);
    signal(SIGCHLD,signal_handler);
   while(1)
   {
        printf("%s",prompt_str);
        fflush(stdout);
        if(scanf("%[^\n]",input_str)!=1)
        {
            getchar();
            continue;
        }
        getchar();

        //step1: check input_str contains "ps1=" or not
        if(strncmp(input_str,"ps1=",4)==0)
        {
            //step2: if it is ps1=
            //check if input_str[4] is space or not
            if(input_str[4] != ' ')
            {
                //if no space
                strcpy(prompt_str,input_str+4);
            }
            else
            {
                //if space is present  print error message
                printf("ERROR: Space not allowed after PS1=\n");
            }
        }
        else
        {
            //if it is not a PS1=
                char *cmd=get_command(input_str);
                int ret=check_command_type(cmd);

                if(ret== BUILTIN)
                {
                    //logic for internal cmd
                    printf("%s is an Builtin Command\n",cmd);
                    execute_internal_commands(input_str);
                }
                else if(ret==EXTERNAL)
                {
                    //logic for external commands
                    
                    printf("%s is an External Command\n",cmd);
                    //before excuting create child processs bcz if we use excec deirectly it will overwrite
                    
                      pid=fork();
                      if(pid>0)
                      {
                        
                        //parent
                        //waitpid()//parent should know that particular process is terminated
                        waitpid(pid,&status,WUNTRACED);//if child process  stop also untraced will know
                        if(WIFSTOPPED(status))
                        {
                            insert_at_first(&head,pid,input_str);
                            pid = 0;
                        }
                        else if(WIFEXITED(status) || WIFSIGNALED(status))
                        {
                            pid = 0;
                        }
                      }
                      else if(pid==0)
                      {
                        signal(SIGINT,SIG_DFL);
                        signal(SIGTSTP,SIG_DFL);//bcz with command if we presse cntrl c or cntrl z it should act as default (external command for internal no need )
                        execute_external_commands(input_str);
                        //exit(0);
                      }
                }
                
                else
                {
                    //print error msg
                    printf("%s : Command not found\n",cmd);
                }

        }

    }
}