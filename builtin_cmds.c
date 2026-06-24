/*builtin commands*/

#include "shell.h"

char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
						"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
						"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help","jobs","fg","bg", NULL};

extern char *external_commands[154];
extern int status;
extern int pid;
extern Slist *head;
void print_list(Slist *head)
{
	if(head!=NULL)
    {
	    while (head)		
	    {
		    printf("Stopped          %s\n", head ->command);
		    head = head -> link;
	    }

    }
}
int delete_first(Slist **head)
{
    
    //list is not empty
	if(*head!=NULL)
    {
		Slist *t=*head;
    	*head=t->link;//updating head address
    	free(t); //deleting node
	}
    return 1;

}
void extract_external_commands(char **external_commands)
{
	//open a file and read content till \n
	//take length of particular command
	//allocate the col memory based on the length
	//store the command in external_commands[][] array
	//repeat it for all the command
	int fd = open("external_cmds.txt",O_RDONLY);
	if(fd == -1)
	{
		perror("open");
    	return;
	}
	char cmd[100];
	char ch;
	int i=0,j=0;
	while(read(fd,&ch,1) > 0)
	{
		if(ch == '\n')
		{
			cmd[j]= '\0';
			external_commands[i] = malloc(strlen(cmd)+1);
			if(external_commands[i]!=NULL)
			{
				strcpy(external_commands[i],cmd);
			}
			i++;
			j=0;
		}
		else if(ch == '\r')
		{
    		continue;
		}
		else
		{
			cmd[j++] = ch;
		}
	}
	if(j > 0)
	{
		cmd[j]= '\0';
			external_commands[i] = malloc(strlen(cmd)+1);
			if(external_commands[i]!=NULL)
			{
				strcpy(external_commands[i],cmd);
			}
			i++;
	}
	external_commands[i]=NULL;
	close(fd);
	// for(int i=0;external_commands[i]!=NULL;i++)
	// printf("%s \n",external_commands[i]);
}

char *get_command(char *input_str)
{
	static char cmd[100];
	//fetch the first word ->input_str => ls -l and store it in cmd array
	sscanf(input_str,"%s",cmd);
	return cmd;
}

int check_command_type(char *cmd)
{
	//compare cmd[] with buitin[][] arr
	for(int i=0;builtins[i]!=NULL;i++)
	{
		//if it is matching
		if(strcmp(cmd,builtins[i])==0)
		return BUILTIN;
	}

	//compare cmd[] with external_command[][] arr
	for(int i=0;external_commands[i]!=NULL;i++)
	{
		//if it is matching
		if(strcmp(external_commands[i],cmd) == 0)
		{
			return EXTERNAL;
		}
	}

	return NO_COMMAND;
}
void execute_internal_commands(char *input_str)
{
	/*1. exit
	  2 .cd-used to change directory
	 3. pwd
	*/
//check inout string contain exit or not
	if(strcmp(input_str,"exit")==0)
	{//if it is exit
    	exit(0);
	}
	//if it conatin cd or not
	else if(strncmp(input_str,"cd",2)==0)
	 {//if it is matching 
	        //chdir(const char *path);used to change the directory
			chdir(input_str+3);//+3 bcz we need to skip cd only access the path ex:cd ecep
	 }

	//check if input str is pwd or not-> char *getcwd(char buf[.size], size_t size); it is used to get current working directory
	else if(strcmp(input_str,"pwd")==0)		
	{
		char buf[50];
		getcwd(buf,50);
		//printf the buffer to get current  directory
		printf("%s\n",buf);
	}
	
	//check the input string contains echo $$ or not
	else if(strcmp(input_str,"echo $$")==0)
	{
		//if it is true 
			printf("%d\n",getpid());
	}
	//check the input str contains echo $? or not
	else if(strcmp(input_str,"echo $?")==0)
	{
		//if it is true  print
		printf("%d\n",WEXITSTATUS(status));
	}

	//check the input string contains echo $SHELL or not
	else if(strcmp(input_str,"echo $SHELL")==0)
	{
	//if it is true
		printf("%s\n",getenv("SHELL"));
	}
	
	//check input string contains jobs or not
	else if(strcmp(input_str,"jobs")==0)
	{
		//it is true
	 	print_list(head);
	}

	//check input_str contains fg or not
	else if(strcmp(input_str,"fg")==0)
	{
		//if it is true
		if(head!=NULL)
		{
			kill(head->pid,SIGCONT);
			waitpid(head->pid,&status,WUNTRACED);
			if(WIFSTOPPED(status))
			{
    			return;   // still stopped, keep it in list
			}
			else
			{
    			delete_first(&head);
			}
			
		}
	}

	//check input_str contains bg or not
	else if(strcmp(input_str,"bg")==0)
	{
		
		//if it is true
		if(head!=NULL)
		{
			kill(head->pid,SIGCONT);
			delete_first(&head);

			//signal(SIGCHLD,signal_handler);
		}
	}

}
void execute_external_commands(char *input_str)
{
	char *arr[50];
	char cmd[100];
	int i=0;
	int j=0,k=0;
	//convert 1d array 2d(arr[][])array ex ls -l whenver there is a space increment the row ar[0]=ls arr[1]=-l arr[2]=NULL;
	while(input_str[i]!='\0')
	{
		if(input_str[i] == ' ')
		{
			cmd[k]= '\0';
			arr[j] = malloc(strlen(cmd)+1);
			if(arr[j]!=NULL)
			{
				strcpy(arr[j],cmd);
			}
			j++;
			k=0;
		}
		else
		{
			cmd[k++] = input_str[i];
		}
		i++;
	}
	if(k > 0)
	{
		cmd[k]= '\0';
			arr[j] = malloc(strlen(cmd)+1);
			if(arr[j]!=NULL)
			{
				strcpy(arr[j],cmd);
			}
			j++;
	}
	arr[j]=NULL;
	int cmd_pos[j];
	int pipe_count=0;
	cmd_pos[0]=0;
	//check if pipe is present or not
	for(int i=0;i<j;i++)
	{
		if(strcmp(arr[i],"|")==0)
		{
			pipe_count++;
			arr[i]=NULL;
			cmd_pos[pipe_count]=i+1;
		}
	}
	if(pipe_count==0)
	{
		//if there is no pipe use
	     execvp(arr[0],arr);
	}
	else
	{
		//if pipe is present
		//n pipe logic 


    int prev_fd = -1;

    for(int i = 0; i < pipe_count + 1; i++)
    {
        int fd[2];

        if(i != pipe_count)
        {
            pipe(fd);
        }

        pid_t pid = fork();

        if(pid == 0)
        {
            if(prev_fd != -1)
            {
                dup2(prev_fd, 0);
                close(prev_fd);
            }

            if(i != pipe_count)
            {
                close(fd[0]);
                dup2(fd[1], 1);
                close(fd[1]);
            }

            execvp(arr[cmd_pos[i]], arr + cmd_pos[i]);
            perror("execvp");
            exit(1);
        }

        if(prev_fd != -1)
        {
            close(prev_fd);
        }

        if(i != pipe_count)
        {
            close(fd[1]);
            prev_fd = fd[0];
        }
    }

    for(int i = 0; i < pipe_count + 1; i++)
    {
        wait(NULL);
    }

	}
}