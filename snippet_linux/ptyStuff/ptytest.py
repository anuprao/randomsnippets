import os
import sys
import time
import pty
import subprocess
import select
import tty
import signal
import array
import fcntl
import termios

master_fd = None
slave_fd = None

def executeChild():
	global master_fd
	os.close(master_fd)
	
	# print "In Child"
	
	os.dup2(slave_fd, pty.STDERR_FILENO)
	
	cmd = 'bash'
	p = subprocess.Popen(cmd, stdin = slave_fd, stdout = slave_fd)
	print "In Child"
	
def executeParent():
	global slave_fd
	os.close(slave_fd)
	
	# print "In Parent"
	
	master_fdobj = os.fdopen(master_fd, 'w+')
	master_select = select.poll()
	master_select.register(master_fdobj, select.POLLIN) # | select.POLLOUT)
	master_select.register(sys.stdin, select.POLLIN)
	
	stdin_fd = sys.stdin.fileno()
	tty.setraw(stdin_fd)
	stdout_fd = sys.stdout.fileno()
	
	while True:
		l = master_select.poll(-1)
		if l:
			for i in l:
				if stdin_fd == i[0]:
					if select.POLLIN & i[1]:
						data = os.read(stdin_fd, 1024)
						while data != '':
							n = os.write(master_fd, data)
							data = data[n:]	
						#master_fdobj.flush()					
					
				if master_fd == i[0]:
					if select.POLLIN & i[1]:
						data = os.read(master_fd, 1024)
						while data != '':
							n = os.write(stdout_fd, data)
							data = data[n:]	
						#sys.stdout.flush()										
	
	master_select.close()	
	
def _signal_winch(signum, frame):
	global master_fd
	# Get the terminal size of the real terminal, set it on the pseudoterminal.
	buf = array.array('h', [0, 0, 0, 0])
	fcntl.ioctl(pty.STDOUT_FILENO, termios.TIOCGWINSZ, buf, True)
	fcntl.ioctl(master_fd, termios.TIOCSWINSZ, buf)
			
if __name__ == "__main__":		
	old_handler = signal.signal(signal.SIGWINCH, _signal_winch)
	master_fd, slave_fd = pty.openpty()
	child_pid = os.fork()
	if child_pid == 0:
		#print "Child Process: PID# %s" % os.getpid()
		executeChild()
	else:
		#print "Parent Process: PID# %s" % os.getpid()
		executeParent()
	signal.signal(signal.SIGWINCH, old_handler)
	print "exiting : ", child_pid
