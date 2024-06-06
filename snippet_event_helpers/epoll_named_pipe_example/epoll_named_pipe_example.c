// gcc -o epoll_named_pipe_example epoll_named_pipe_example.c

#include <stdio.h>     
#include <stdlib.h>
#include <string.h>      
#include <errno.h>
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
		//

		int epoll_fd ;

		printf("Hello from Parent ... pid=%d!\n", getpid());  

		//

		epoll_fd = epoll_create1(0);
	
		if(epoll_fd == -1)
		{
			fprintf(stderr, "Failed to create epoll file descriptor\n");
			return 1;
		}		

		{
			int i;
			int quit = 0;

			int event_count;
			struct epoll_event newEvents[MAX_EVENTS];

			unsigned char strRecd[128];
			
			// parent process because return value non-zero. 

			//
			
			fd_input_write = open(FN_INPUT_WRITE, O_WRONLY);

			//close(fd_input_read); 		// file descriptor unused in parent
			
			fd_output_read = open(FN_OUTPUT_READ, O_RDONLY);
			
			//close(fd_output_write); 		// file descriptor unused in child	

			//


			//
			{
				struct epoll_event epollEvent_KB;

				epollEvent_KB.events = EPOLLIN;
				epollEvent_KB.data.fd = fd_output_read;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_output_read, &epollEvent_KB) == -1)
				{
					printf("epoll_ctl(ADD) failed: errno=%d\n", errno);
					close(epoll_fd);
					exit(10);
				}				
			}

			//

			while(0 == quit)
			{	
				//printf("In poll_wait ... \n");
				// Running epoll_wait with empty list works like a simple delay
				memset(&newEvents, 0, sizeof(newEvents));
				event_count = epoll_wait(epoll_fd, newEvents, MAX_EVENTS, -1);
				
				//printf("%d ready events\n", event_count);
				for(i = 0; i < event_count; i++)
				{
					//printf("newEvents[i].events : %d\n", newEvents[i].events);

					if(newEvents[i].events & EPOLLIN )
					{
						// Read FDs in Event list to reset them and prevent repeat triggering
						if(fd_output_read == newEvents[i].data.fd)
						{
							int n;

							do
							{
								memset(strRecd,0,sizeof(strRecd));
								n = read(fd_output_read, strRecd, sizeof(strRecd));
								if (n > 0) 
								{
									//printf("Recd [%d]: %s\n", n, strRecd);
									printf("%s", strRecd);
									//puts(strRecd);
								}
							} while (n > 0);
						}
					}

					//printf("EPOLLHUP | EPOLLOUT | EPOLLPRI : %d\n", EPOLLHUP | EPOLLOUT | EPOLLPRI);
					//if(newEvents[i].events & (EPOLLHUP | EPOLLOUT | EPOLLPRI))
					if(newEvents[i].events & (EPOLLHUP | EPOLLPRI))
					{
						quit = 1;
					}
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

