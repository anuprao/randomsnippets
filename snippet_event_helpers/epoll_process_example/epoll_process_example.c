// gcc -o epoll_process_example epoll_process_example.c 

#include <stdio.h>     
#include <stdlib.h>
#include <string.h>   

#include <errno.h>
#include <unistd.h>    
#include <pty.h>
#include <termios.h>

#include <sys/epoll.h> 

#define MAX_EVENTS 5
#define READ_SIZE 10

struct termios prev_tty_cfg;

int fdPtyMaster = -1;

int main()
{
	char* const args[] = {"python3", "test.py", NULL}; 
	//char* const args[] = {"ls", "/mnt/workspace", NULL}; 

	pid_t pid; 

	//

	//pid = fork(); 
	pid = forkpty(&fdPtyMaster, NULL, NULL, NULL);
	if(0 > pid) 
	{ 
		perror("fork fail"); 
		exit(1); 
	} 
	
	else if (0 == pid) 
	{
		// child process because return value zero 
		//printf("Hello from Child ... pid=%d!\n", getpid()); 

		{
			//printf("Child process execution !!\n");

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

		//

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

			struct termios tios;

			// parent process because return value non-zero. 
			
			//
			{
				struct epoll_event epollEvent_KB;

				epollEvent_KB.events = EPOLLIN;
				epollEvent_KB.data.fd = fdPtyMaster ;//fds_out[0];
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fdPtyMaster, &epollEvent_KB) == -1)
				{
					printf("epoll_ctl(ADD) failed: errno=%d\n", errno);
					close(epoll_fd);
					exit(10);
				}				
			}

			tcgetattr(fdPtyMaster, &tios);
			tios.c_lflag &= ~(ECHO | ECHONL);
			tcsetattr(fdPtyMaster, TCSAFLUSH, &tios);

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
						if(fdPtyMaster == newEvents[i].data.fd)
						{
							int n;

							memset(strRecd,0,sizeof(strRecd));
							n = read(fdPtyMaster, strRecd, sizeof(strRecd));
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

			// send EOF so child can continue (child blocks until all input has been processed):
			//close(fds_in[1]);

			//close(fds_out[0]);
		}

		printf("Parent done ... !\n"); 
	}
}