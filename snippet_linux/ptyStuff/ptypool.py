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
import curses
import curses.ascii

class session:
	def __init__(self, jobname, cmdline):
		self.master_fd = None
		self.slave_fd = None
		self.jobname = jobname
		self.cmdline = cmdline
		self.master_fdobj = None
		self.child_pid = None
		
	def setup(self):
		self.master_fd, self.slave_fd = pty.openpty()
		self.child_pid = os.fork()		
		return self.child_pid, self.master_fd
		
	def preparePolling(self, master_select):
		#print self.slave_fd
		os.close(self.slave_fd)
		self.slave_fd = None
		self.master_fdobj = os.fdopen(self.master_fd, 'w+')
		master_select.register(self.master_fdobj, select.POLLIN)
				
	def executeChild(self):
		os.close(self.master_fd)		
		os.dup2(self.slave_fd, pty.STDERR_FILENO)
		p = subprocess.Popen(self.cmdline, stdin = self.slave_fd, stdout = self.slave_fd)
		sys.exit()
		
class sessionhandler:
	def __init__(self):
		self.dictSessions = {}
		self.arrayMfds = []
		self.current_fd = None
		self.bParent = True
		self.master_select = None
	
	def on_SIGWINCH(self, signum, frame):
		buf = array.array('h', [0, 0, 0, 0])
		fcntl.ioctl(pty.STDOUT_FILENO, termios.TIOCGWINSZ, buf, True)
		for mfd in self.arrayMfds:
			fcntl.ioctl(mfd, termios.TIOCSWINSZ, buf)
	
	def addNewSession(self, newSession):		
		pid, mfd = newSession.setup()
		#print "pid, mfd :", pid, mfd
		if 0 == pid :
			self.bParent = False
			newSession.executeChild()
		else:
			self.arrayMfds.append(mfd)
			nlen = len(self.dictSessions)
			self.dictSessions[nlen] = newSession
			newSession.preparePolling(self.master_select)
				
	def transactWithChild(self):		
		bLoop = True
		while bLoop:
			l = self.master_select.poll(-1)
			for i in l:
				if stdin_fd == i[0]:
					if select.POLLIN & i[1]:
						data = os.read(stdin_fd, 1)
						if 0x01 == ord(curses.ascii.ctrl(data)) and not curses.ascii.isalpha(data) :
							bLoop = False
						else:
							while data != '':
								n = os.write(self.current_fd, data)
								data = data[n:]					
				
				if self.current_fd == i[0]:
					if select.POLLIN & i[1]:
						data = os.read(self.current_fd, 1024)
						while data != '':
							n = os.write(stdout_fd, data)
							data = data[n:]										

if __name__ == "__main__":	
	oSh = sessionhandler()
	if True == oSh.bParent:	
		
		old_handler = signal.signal(signal.SIGWINCH, oSh.on_SIGWINCH)
		stdin_fd = sys.stdin.fileno()
		tty.setraw(stdin_fd)
		stdout_fd = sys.stdout.fileno()
		oSh.master_select = select.poll()
		oSh.master_select.register(sys.stdin, select.POLLIN)		
				
		jobname = None
		cmdline = None
		inp = 0
		
		while inp != ord('q'):
			screen = curses.initscr()
			screen.clear()
			
			y = 3
			screen.addstr( y, 3, "a - Add a job")
			y = y + 1
			screen.addstr( y, 3, "q - Quit")
			y = y + 1
			
			y = y + 1
			
			for key,item in oSh.dictSessions.items():
				screen.addstr( y, 3, str(key) + " - " + item.jobname)
				y = y + 1
			
			screen.addstr( 1, 1, "Choose an option from below :")
			
			screen.refresh()

			inp = screen.getch()
			
			if inp == ord('a'):
				screen.addstr( y, 3, "Job name: ")
				screen.refresh()
				jobname = screen.getstr(y, 13, 53)
				y = y + 1
				
				screen.addstr( y, 3, "Command: ")
				screen.refresh()
				cmdline = screen.getstr(y, 13, 53)
				y = y + 1		
				
				curses.endwin()
				
				newSession = session(jobname, cmdline)
				oSh.addNewSession(newSession)
						
			if  ord('0')<=inp and inp<=ord('9')  :
				curses.endwin()	
				index = inp - ord('0') 
				
				oSh.current_fd = oSh.arrayMfds[index]
				
				buf = array.array('h', [0, 0, 0, 0])
				fcntl.ioctl(pty.STDOUT_FILENO, termios.TIOCGWINSZ, buf, True)
				fcntl.ioctl(oSh.current_fd, termios.TIOCSWINSZ, buf)
				
				os.kill(oSh.dictSessions[index].child_pid, signal.SIGWINCH)
				
				os.write(oSh.current_fd, '\f')
				
				oSh.transactWithChild()
		
		curses.endwin()
		signal.signal(signal.SIGWINCH, old_handler)	
		#oSh.master_select.close()
	
	'''
	print "inp :",inp
	print "jobname :",jobname
	print "cmdline :",cmdline
	'''
