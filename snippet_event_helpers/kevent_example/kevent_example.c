//Ref: https://gist.github.com/lichray/3100196
//Ref: https://wiki.netbsd.org/tutorials/kqueue_tutorial/

/* Copyright (C) 2002 by Jilles Tjoelker */
/* revised by Zhihao Yuan, 2012 */

#include	<sys/types.h>
#include	<sys/event.h>
#include	<sys/time.h>

#include	<err.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdio.h>
#include	<unistd.h>


int
main(int argc, char *argv[])
{
    char buf[1024];
    int kq;
    struct kevent ev[1];
    struct timespec ts = { 5, 0 };
    ssize_t n;

    if (argc != 1)
    {
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return(1);
    }

    kq = kqueue();
    if (kq == -1)
		err(1,"kqueue");

    EV_SET(ev, STDIN_FILENO, EVFILT_READ, EV_ADD, 0, 0, NULL);
    
    if (kevent(kq, ev, 1, NULL, 0, &ts) == -1)
		err(1,"setup kevent");

    for (;;)
    {
		switch (kevent(kq, NULL, 0, ev, 1, &ts))
		{
			case 0:
				printf("timeout expired\n");
				break;
			
			case -1:
				err(1, "kevent");
				break;
			
			default:
				printf("data ready (%ld)\n", ev->data);
				n = read(STDIN_FILENO, buf, sizeof buf - 1);
				buf[n] = '\0';
				printf("(%zd) %s\n", n, buf);
		}
    }

    close(kq);

    return 0;
}
