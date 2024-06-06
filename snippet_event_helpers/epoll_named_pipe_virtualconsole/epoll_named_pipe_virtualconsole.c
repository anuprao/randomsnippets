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
	int fd_input_read = -1;
	int fd_output_write = -1;
		
	//

	{
		//

		int epoll_fd ;

		printf("Hello from Virtual Console ... pid=%d!\n", getpid());  

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

			const char* pStrCallout = "Hello Console !";
			
			// parent process because return value non-zero. 

			//
			
			fd_input_read = open(FN_INPUT_READ, O_RDONLY | O_NONBLOCK);
			if(0 < fd_input_read)
			{
				printf("Opened  fd_input_read = %d\n", fd_input_read);
			}
			else
			{
				perror("Could not open fd_input_read");
			}
			
			// USe RDWR since, as WRONLY can only be used if the pipe is known to be aleady opened for reading
			fd_output_write = open(FN_OUTPUT_WRITE, O_RDWR | O_NONBLOCK);
			if(0 < fd_output_write)
			{			
				printf("Opened  fd_output_write = %d\n", fd_output_write);
			}
			else
			{
				perror("Could not open fd_output_write");
			}		

			//
			{
				struct epoll_event epollEvent_KB;

				epollEvent_KB.events = EPOLLIN;
				epollEvent_KB.data.fd = fd_input_read;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_input_read, &epollEvent_KB) == -1)
				{
					printf("epoll_ctl(ADD) failed: errno=%d\n", errno);
					close(epoll_fd);
					exit(10);
				}				
			}

			//

			while(0 == quit)
			{	
				printf("In poll_wait ... \n");

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
						if(fd_input_read == newEvents[i].data.fd)
						{
							int n;

							memset(strRecd,0,sizeof(strRecd));
							n = read(fd_input_read, strRecd, sizeof(strRecd));
							if (n > 0) 
							{
								printf("Recd [%d]: %s\n", n, strRecd);

								if(0 == strcmp(strRecd, pStrCallout));
								{
									printf("Sending ... response\n");

									{
										int nWritten = -1;
										const char* pStrResponse= "Hello Operator !";
										int nLen = strlen(pStrResponse);
										nWritten = write(fd_output_write, pStrResponse, nLen);
										fdatasync(fd_output_write);

										printf("Wrote %d on %d bytes !!!\n", nWritten, fd_output_write);
									}
								}
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
			close(fd_input_read);

			close(fd_output_write);
		}
		
		//

		printf("Virtual Console done ... !\n"); 
	}
}

