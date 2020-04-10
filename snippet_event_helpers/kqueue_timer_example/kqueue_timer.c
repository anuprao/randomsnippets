//Ref: https://wiki.netbsd.org/tutorials/kqueue_tutorial/

#include <sys/event.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
   struct kevent change;    /* event we want to monitor */
   struct kevent event;     /* event that was triggered */
   //pid_t pid;
   int kq, nev;

   /* create a new kernel event queue */
	if ((kq = kqueue()) == -1)
	{
	  fprintf(stderr, "kqueue()\n");
	  fflush(stderr);
	}

   /* initalise kevent structure */
   EV_SET(&change, 1, EVFILT_TIMER, EV_ADD | EV_ENABLE , NOTE_USECONDS, 100, 0);

   /* loop forever */
   for (;;) 
   {
      nev = kevent(kq, &change, 1, &event, 1, NULL);

      if (nev < 0)
      {
		  fprintf(stderr, "Error!\n");
		  fflush(stderr);
	  }

      else if (nev > 0) 
      {
         if (event.flags & EV_ERROR) 
         {   /* report any error */
            fprintf(stderr, "EV_ERROR: %s\n", strerror(event.data));
            exit(EXIT_FAILURE);
         }

         printf(".");
         fflush(stdout);
      }
   }

   close(kq);
   return EXIT_SUCCESS;
}
