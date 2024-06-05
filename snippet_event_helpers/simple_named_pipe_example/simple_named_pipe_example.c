// gcc -o epoll_named_pipe_example epoll_named_pipe_example.c

#include <stdio.h>     
#include <stdlib.h>
#include <string.h>      

#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/epoll.h> 


#define MAX_EVENTS			5
#define READ_SIZE			160

#define FN_INPUT_FIFO		"FIFO_INPUT"
#define FN_INPUT_READ		FN_INPUT_FIFO
#define FN_INPUT_WRITE		FN_INPUT_FIFO

#define FN_OUTPUT_FIFO		"FIFO_OUTPUT"
#define FN_OUTPUT_READ		FN_OUTPUT_FIFO
#define FN_OUTPUT_WRITE		FN_OUTPUT_FIFO

int main()
{
	char* const args[] = {"python3", "test_with_flush.py", NULL}; 

	//int fds_in[2];
	//fds_in[0] : Read end of pipe
	//fds_in[1] : Write end of pipe
	
	//int fds_out[2];
	//fds_out[0] : Read end of pipe
	//fds_out[1] : Write end of pipe
	
	pid_t pid; 

	// pipe(fds_in); 
	// pipe(fds_out); 
	
	int fd_input_read = -1;
	int fd_output_write = -1;

	int fd_input_write = -1;
	int fd_output_read = -1;
		
	//

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
			
			fd_input_read = open(FN_INPUT_READ, O_RDONLY);
			
			fd_output_write = open(FN_OUTPUT_WRITE, O_WRONLY);
			
			//

			dup2(fd_input_read, STDIN_FILENO);    // fd_input_read (the read end of pipe) 
			close(fd_input_read);                 // file descriptor no longer needed in child since stdin is a copy

			//close(fd_input_write);                 // file descriptor unused in child

			//

			dup2(fd_output_write, STDOUT_FILENO);   // fd_output_write (the write end of pipe) 
			close(fd_output_write);                 // file descriptor no longer needed in child since stdout is a copy

			//close(fd_output_read);                 // file descriptor unused in child	

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
			
			//
			
			fd_input_write = open(FN_INPUT_WRITE, O_WRONLY);
			
			fd_output_read = open(FN_OUTPUT_READ, O_RDONLY);
			
			//

			// if we reach here, we are in parent process

			//close(fd_input_read); 		// file descriptor unused in parent

			//close(fd_output_write); 		// file descriptor unused in child	

			//
			for (size_t i = 0; i < 40; i++) 
			{
				memset(strRecd,0,sizeof(strRecd));
				
				n = read(fd_output_read, strRecd, sizeof(strRecd));
				printf("Read %d bytes ...\n", n);
				if(0 < n)
				{
					printf("Recd [%d]: %s\n", n, strRecd);
				}
			}

			//

			// send EOF so child can continue (child blocks until all input has been processed):

			close(fd_input_write);

			close(fd_output_read);
		}
		
		//

		printf("Parent done ... !\n"); 
	}
}

