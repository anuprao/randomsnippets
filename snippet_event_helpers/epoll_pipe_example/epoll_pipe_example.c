#include <stdio.h>     
#include <stdlib.h>
#include <string.h>    
#include <unistd.h>    

#include <sys/epoll.h> 

#define MAX_EVENTS 5
#define READ_SIZE 10

int main()
{
	char* const args[] = {"python3", "test_with_flush.py", NULL}; 

	int fds_in[2];
	int fds_out[2];

	pid_t pid; 

	pipe(fds_in); 

	pipe(fds_out); 

	pid = fork(); 
	if(0 > pid) 
	{ 
		perror("fork fail"); 
		exit(1); 
	} 
	
	else if (0 == pid) 
	{
		// child process because return value zero 
		printf("Hello from Child ... pid=%d!\n", getpid()); 

		{
			printf("Child process execution !!\n");

			//

			dup2(fds_in[0], STDIN_FILENO);    // fds_in[0] (the read end of pipe) 
			close(fds_in[0]);                 // file descriptor no longer needed in child since stdin is a copy

			close(fds_in[1]);                 // file descriptor unused in child

			//

			dup2(fds_out[1], STDOUT_FILENO);   // fds_out[1] (the write end of pipe) 
			close(fds_out[1]);                 // file descriptor no longer needed in child since stdout is a copy

			close(fds_out[0]);                 // file descriptor unused in child	

			//					

			execvp(args[0], args); 
		}

		printf("Alert ! Child process did nnot execute !!\n");
		exit(1);
	}
	
	else
	{
		printf("Hello from Parent ... pid=%d!\n", getpid());  

		{
			int n;
			char strRecd[512];
			// parent process because return value non-zero. 
			

			// if we reach here, we are in parent process

			close(fds_in[0]); // file descriptor unused in parent

			close(fds_out[1]); // file descriptor unused in child	

			//

			do
			{
				memset(strRecd,0,sizeof(strRecd));
				n = read(fds_out[0], strRecd, sizeof(strRecd));
				if (n > 0) 
				{
					//printf("Recd [%d]: %s\n", n, strRecd);
					printf("%s", strRecd);
					//puts(strRecd);
				}
			} while (n > 0);

			//

			// send EOF so child can continue (child blocks until all input has been processed):

			close(fds_in[1]);

			close(fds_out[0]);
		}
		
		//

		printf("Parent done ... !\n"); 
	}
}
