#include <stdio.h>     
#include <stdlib.h>
#include <string.h>    
#include <unistd.h>    

#include <sys/epoll.h> 

#define MAX_EVENTS 5
#define READ_SIZE 10

int main2()
{
	int running = 1, event_count, i;
	size_t bytes_read;
	char read_buffer[READ_SIZE + 1];
	struct epoll_event new_event, obs_events[MAX_EVENTS];
	int epoll_fd = epoll_create1(0);
 
	if(epoll_fd == -1)
	{
		fprintf(stderr, "Failed to create epoll file descriptor\n");
		return 1;
	}

	// Skip the fllowing two lines to make code work like a simple delay
	new_event.events = EPOLLIN;
	new_event.data.fd = 0;
 
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &new_event))
	{
		fprintf(stderr, "Failed to add file descriptor to epoll\n");
		close(epoll_fd);
		
		return 1;
	}
 
	while(running)
	{
		fprintf(stdout, ".");
		fflush(stdout);
		
		// Running epoll_wait with empty list works like a simple delay
		event_count = epoll_wait(epoll_fd, obs_events, MAX_EVENTS, 1);
		
		//printf("%d ready obs_events\n", event_count);
		for(i = 0; i < event_count; i++)
		{
			printf("Reading file descriptor '%d' -- ", obs_events[i].data.fd);
			bytes_read = read(obs_events[i].data.fd, read_buffer, READ_SIZE);
			
			printf("%zd bytes read.\n", bytes_read);
			read_buffer[bytes_read] = '\0';
			
			printf("Read '%s'\n", read_buffer);
 
			if(!strncmp(read_buffer, "stop\n", 5))
				running = 0;
		}
	}
 
	if(close(epoll_fd))
	{
		fprintf(stderr, "Failed to close epoll file descriptor\n");
		return 1;
	}
	return 0;
}

/*
int main2()
{
	pid_t pid; 

	pid = fork(); 
	if(pid<0) 
	{ 
		perror("fork fail"); 
		exit(1); 
	} 
	
	else if ( pid == 0) 
	{
		char* const args[] = {"python3", "test.py", NULL}; 

		// child process because return value zero 
		printf("Hello from Child ... !\n"); 

		execvp(args[0], args); 

		printf("Alert ! Check child process execution !!\n");
		exit(1);
	}
	
	else
	{
		// parent process because return value non-zero. 
		printf("Hello from Parent ... !\n"); 
	}
}
*/

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
			
			for (size_t i = 0; i < 40; i++) 
			{
				memset(strRecd,0,sizeof(strRecd));
				
				n = read(fds_out[0], strRecd, sizeof(strRecd));
				printf("Read %d bytes ...\n", n);
				if(0 < n)
				{
					printf("Recd [%d]: %s\n", n, strRecd);
				}
			}

			//

			// send EOF so child can continue (child blocks until all input has been processed):

			close(fds_in[1]);

			close(fds_out[0]);
		}
		
		//

		printf("Parent done ... !\n"); 
	}
}
