epoll_named_pipe_virtualconsole: epoll_named_pipe_virtualconsole.c
	gcc -Wall -Werror -o $@ epoll_named_pipe_virtualconsole.c

epoll_named_pipe_virtualoperator: epoll_named_pipe_virtualoperator.c
	gcc -Wall -Werror -o $@ epoll_named_pipe_virtualoperator.c

epoll_named_pipe_exec_redirect: epoll_named_pipe_exec_redirect.c
	gcc -Wall -Werror -o $@ epoll_named_pipe_exec_redirect.c
		 
clean:
	@rm -v epoll_named_pipe_virtualconsole epoll_named_pipe_virtualoperator epoll_named_pipe_exec_redirect

all: epoll_named_pipe_virtualconsole epoll_named_pipe_virtualoperator epoll_named_pipe_exec_redirect


