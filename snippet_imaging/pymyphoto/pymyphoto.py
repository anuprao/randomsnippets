#!/usr/bin/python

import os
import os.path
from PIL import Image

inputFolder = "input"
outputFolder = "output"

dirSrcFilePaths = {}

def AssessNewSize(oldW,oldH):
	newW = oldW
	newH = oldH
	
	if newH >= 800 :
		newW = (800*oldW)/oldH
		newH = 800
	elif newH < 800 and newH >= 600 :
		newW = (600*oldW)/oldH
		newH = 600	
	elif newH < 600 and newH >= 480 :
		newW = (480*oldW)/oldH
		newH = 480	
	elif newH < 480 and newH >= 360 :
		newW = (360*oldW)/oldH
		newH = 360
	
	oldW = newW
	oldH = newH	
	
	if newW >= 1280 :
		newH = (1280*oldH)/oldW
		newW = 1280	
	elif newW < 1280 and newW >= 1024:
		newH = (1024*oldH)/oldW
		newW = 1024		
	elif newW < 1024 and newW >= 800:
		newH = (800*oldH)/oldW
		newW = 800	
	elif newW < 800 and newW >= 640:
		newH = (640*oldH)/oldW
		newW = 640	
	elif newW < 640 and newW >= 480:
		newH = (480*oldH)/oldW
		newW = 480	
	
	return newW,newH

def CollectFilenames(pathPhotos):
	global dirSrcFilePaths
	listSrcFilePaths = None
	
	fullpathPhotos = os.path.join(inputFolder,pathPhotos)
	print "Collecting names from %s" %(fullpathPhotos)
	
	if not os.path.isdir(fullpathPhotos):
		print "Please specify folder"
		return
		
	print "Collecting filenames"
	listSrcFilePaths = os.listdir(fullpathPhotos)
	
	if None != listSrcFilePaths:
		dirSrcFilePaths[pathPhotos]	= listSrcFilePaths		

def ResizePhotos():
	if not os.path.exists(outputFolder):
		os.mkdir(outputFolder)
	
	for pathPhotos in dirSrcFilePaths.keys():
		print "Processing %s" %(pathPhotos)
		dstfolder = os.path.join(outputFolder,pathPhotos)
		if not os.path.exists(dstfolder):
			os.mkdir(dstfolder)		
		print "Adding to %s" %(dstfolder)
		for pathSrcFile in dirSrcFilePaths[pathPhotos]:
			im = Image.open(os.path.join(inputFolder,pathPhotos,pathSrcFile))
			w,h = im.size
			newsize = AssessNewSize(w,h)
			newim = im.resize(newsize,Image.ANTIALIAS)
			pathDstFile = os.path.join(dstfolder,pathSrcFile)
			print pathDstFile
			#newim.save(pathDstFile)
			newim.save(pathDstFile,quality=72)
		

if __name__ == "__main__":
	print "Starting pymyphoto..."
	
	#CollectFilenames("Anju")	
	CollectFilenames("MyBirthDay2")
	#CollectFilenames("Dad")
	#CollectFilenames("Family")
	#CollectFilenames("Mom")
	#CollectFilenames("Wedding")
	
	print "Resizing photos"
	ResizePhotos()
	
	print "Program has completed"
