#
# asciinema rec session.cast -c byobu
#

# -*- coding: utf-8 -*-

import base64

if __name__ == "__main__":
	
	ft = ""
	
	strSeg = "<html>\n"
	ft = ft + strSeg
	
	strSeg = "\t<head>\n"
	ft = ft + strSeg
	
	strSeg = "\t\t<style>\n"
	ft = ft + strSeg
	
	#'''
	fname = 'asciinema-player.css'
	fp = open(fname,"r")
	strSegFile = fp.read()
	fp.close()
	ft = ft + strSegFile
	#'''

	strSeg = "\t\t</style>\n"
	ft = ft + strSeg
	
	strSeg = "\t</head>\n"
	ft = ft + strSeg
	
	
	fname = 'session.cast'
	fp = open(fname,"r")
	strSegFile = fp.read()
	fp.close()
	
	strSegFile_bytes = strSegFile.encode('utf-8')
	base64_bytes = base64.b64encode(strSegFile_bytes)
	strSegFile_b64 = base64_bytes.decode('ascii')
	
	strSeg = '\t<asciinema-player src="data:text/plain;base64,' + strSegFile_b64 + '">\n'
	ft = ft + strSeg
	
	strSeg = "\t</asciinema-player>\n"
	ft = ft + strSeg
	
	strSeg = "\t<body>\n"
	ft = ft + strSeg
	
	#strSeg = "</asciinema-player><script>"
	#ft = ft + strSeg
	
	strSeg = "\t\t<script>\n"
	ft = ft + strSeg
	
	#'''
	fname = 'asciinema-player.js'
	fp = open(fname,"r")
	strSegFile = fp.read()
	fp.close()
	ft = ft + strSegFile
	#'''
	
	strSeg = "\t\t</script>\n"
	ft = ft + strSeg
	
	strSeg = "\t</body>\n"
	ft = ft + strSeg

	strSeg = "</html>\n"
	ft = ft + strSeg
		
	#
	
	fname = "session.html"
	fp = open(fname,"w")
	fp.write(ft)
	fp.close()
	
	
