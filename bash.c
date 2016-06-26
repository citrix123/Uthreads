#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define true 1
#define false 0

struct command {
	char command[10];
	char arg[5];
};

void shell_start()
{
	char prompt[100];
	char *PWD;
	strcpy(prompt, "My-Shell: ");
	PWD = get_current_dir_name();	
	strcat(prompt, PWD);
	strcat(prompt, " >");
	printf("%s ", prompt);	
}


void parse_string(char *cmd)
{
	char *envp[] = {NULL};
	char *command, *args, *p, buff[10] = "/bin/";
	command = cmd;	 
        while(isspace(*command)) 
        command++;
        p = strchr(command, ' '); 
        if (p) 
        { 
                *p = 0; args = p + 1;
		printf("%d \n" , (int)strlen(args));
                while (isspace(*args))
                        args++;

        }        
        else 
        {
                args = NULL;
        }
					
		strcat(buff, cmd);
		char *argv[] = {buff, args, NULL};
		char *n = "&";	
		if(args)
		{	
			if(p = strstr(args, n))
			{
		
				printf("i got it %s\n", p);
				n = p-1;
				n[0] = 0;
				
				if(fork() == 0)
				{
					execvp(argv[0], argv);
					printf("unknown command \n");
					exit(0);	
				}
				else
				{
					return;					
				}

			}
			//here is the way to go if & is not there but args is there
			else
			{
				//do fork and run the execv command
				pid_t pid;
				int child_status;
				pid = fork();
				if(pid == 0)
				{
					execvp(argv[0], argv);
					printf("unknown command \n");
					exit(0);	
				}	
				if(pid == -1)
					perror("error");
				
				else {
				 /* This is run by the parent.  Wait for the child
					to terminate. */

						pid_t tpid = wait(&child_status);
	     		 do 
					{
							if(tpid !=pid) break;
					} while(tpid != pid);

				//		 return child_status;
				}
			}
		}
		//if args is not present then for and execute and wait for parent
		else
		{
			pid_t pid;
			int child_status;
			pid = fork();
			if(pid == 0)
			{
				execvp(argv[0], argv);
				printf("unknown command \n");
				exit(0);	
			}	
			if(pid == -1)
				perror("error");
			
			else {
			 /* This is run by the parent.  Wait for the child
				to terminate. */

					pid_t tpid = wait(&child_status);
						do 
				{
						if(tpid !=pid) break;
				} while(tpid != pid);

				//	 return child_status;
			}
		}		
	
}
int main()
{
	char cmd[18];		
	
	time_t result = time(NULL);
        printf("Hello you are logged in at %s from This terminal \n  Autor : Rahul Jain\n", asctime(localtime(&result)));
	
	while(1)
	{
		shell_start();
		fgets(cmd,18, stdin);
                cmd[strlen(cmd) - 1] = 0;
		
		if(strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0)
                {
                        printf("Bye.. \n");
                        break;
                }

		parse_string(cmd);
	}

	return 0;
}


