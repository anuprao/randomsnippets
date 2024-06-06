// gcc -o epoll_named_pipe_virtualconsole epoll_named_pipe_virtualconsole.c

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
	int fd_output_read = -1;	
	int fd_input_write = -1;

	//

	{
		//

		int epoll_fd ;

		printf("Hello from Virtual Operator ... pid=%d!\n", getpid());  

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
			
			fd_output_read = open(FN_OUTPUT_READ, O_RDONLY | O_NONBLOCK);
			if(0 < fd_output_read)
			{
				printf("Opened  fd_output_read = %d\n", fd_output_read);
			}
			else
			{
				perror("Could not open fd_output_read");
			}

			fd_input_write = open(FN_INPUT_WRITE, O_RDWR | O_NONBLOCK);
			if(0 < fd_input_write)
			{			
				printf("Opened  fd_input_write = %d\n", fd_input_write);
			}
			else
			{
				perror("Could not open fd_input_write");
			}			
			
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

			{
				const char* pStrCallout = "Hello Console !";
				int nLen = strlen(pStrCallout);
				write(fd_input_write, pStrCallout, nLen);
				fdatasync(fd_input_write);
			}

			while(0 == quit)
			{	
				printf("In poll_wait ... \n");

				// Running epoll_wait with empty list works like a simple delay
				memset(&newEvents, 0, sizeof(newEvents));
				event_count = epoll_wait(epoll_fd, newEvents, MAX_EVENTS, -1);
				
				printf("%d ready events\n", event_count);
				for(i = 0; i < event_count; i++)
				{
					//printf("newEvents[i].events : %d\n", newEvents[i].events);

					if(newEvents[i].events & EPOLLIN )
					{
						// Read FDs in Event list to reset them and prevent repeat triggering
						if(fd_output_read == newEvents[i].data.fd)
						{
							int n;

							memset(strRecd,0,sizeof(strRecd));
							n = read(fd_output_read, strRecd, sizeof(strRecd));
							if (n > 0) 
							{
								printf("Recd [%d]: %s\n", n, strRecd);
							}
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

		printf("Virtual Operator done ... !\n"); 
	}
}

