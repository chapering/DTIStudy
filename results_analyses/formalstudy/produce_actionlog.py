#!/usr/bin/env python
'''
read a run log output of current seeding-study program and retrieve the interaction
sequence 

after processing, the output will be organized to the VTK legacy format and dumped to the stdout

Copyright Chapstudio 2010-2011
Haipeng Cai @Aug. 14th, 2011
'''

import os
import sys
import string
import re
from odict import *
from math import fabs
from PointRotate import *

# name of file to parse
g_fns=[]

# hole all streamlines
g_allimfiles=[]
g_bKeepImfile=False

'''data structure:
	[ {taskid:{res:[(singletasktime, did, [(rottimespan, [(time,rx,ry,rz,angle),...]),...],...),...],...}, ...}, ... ]
'''
g_allparticipantinfo=[]

split=string.split

PREPROCESS_FILTER="TASK|rotating around axis|rotating by mouse|rotating finished|Task completed|resolution|focus order|GLAPP"
# keywords for retrieving feature lines
FORTASK="TASK"
FORROTSTART="rotating by mouse"
FORANGLE="rotating around axis"
FORROTEND="rotating finished"
FORRESOLUTION="under seeding resolution of"
FORANSWER="Task completed with Answer"
FORSTART="GLAPP"
FORFBFOCUS="focus order:"

FBNAMES={
		"cst":1, "cg":2, "cc":3, "ifo":4, "ilf":5}

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <run log> \n" % \
			sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fns
	argc = len(sys.argv)
	if argc >= 2:
		for i in range(1, argc):
			g_fns.append( sys.argv[i] )
			if not os.path.exists( g_fns[i-1] ):
				raise IOError, "source file given [%s] does not exist, bailed out now." \
					% g_fns[i-1]
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

## @brief preprocess the raw log and filter out only relevant information 
##	for the statistics
## @param srcfns a list of raw log files
## @return none
def preprocess(srcfns):
	global g_allimfiles
	for rfn in srcfns:
		imfn = "%s_action" % rfn
		os.system("cat %s | grep -E \"%s\" > %s" % (rfn, PREPROCESS_FILTER, imfn) )
		g_allimfiles.append( imfn )
		print >> sys.stderr, "%s processed and procuded %s" % (rfn, imfn)

## @brief load the run log for a single participant
## @param srcfn the run log file of a single participant
## @return none
def loadsingle(srcfn):
	def pickTime( curline ) :
		lasttime = -1
		curline = curline.lstrip().rstrip(' \r\n')
		if len(curline) > 10 and curline[0] == '[':
			timeinfo = split( curline[1:] )
			mssec = int( timeinfo[1] )
			timeinfo = split( timeinfo[0], ':', 2 )
			lasttime = int( timeinfo[0] ) * 3600 + int( timeinfo[1] ) * 60 + int (timeinfo[2]) + mssec*1.0/1000
		return lasttime

	def pickResolution( curline ) :
		resolution = (-1,-1,-1)
		curline = curline.lstrip().rstrip(' \r\n')
		if len (curline) > 10 and string.find( curline, FORRESOLUTION) != -1:
			resinfo = split ( curline[5:-5] )
			if len(resinfo) >= 5:
				resinfo = split ( resinfo[4], 'x', 3 )
				resolution = (resinfo[0], resinfo[1], resinfo[2])
		return resolution

	def pickRotInfo( curline ) :
		ret=()
		curline = curline.lstrip().rstrip(' \r\n')
		if len (curline) > 50 and string.find( curline, FORANGLE) != -1:
			retinfo = split ( curline )
			if len(retinfo) != 11 :
				# not available
				return -2
			rotaxis= retinfo[6].lstrip("(").rstrip(")")
			axisinfo = split( rotaxis, ',', 2 )

			angleinfo = split( retinfo[8], '=', 1 )

			ret=( float(axisinfo[0]),float(axisinfo[1]),float(axisinfo[2]), float(angleinfo[1]) )
		return ret

	def pickTaskId( curline ):
		taskId = -1
		curline = curline.lstrip().rstrip().rstrip('\r\n')
		taskinfo = split( curline )
		if len(taskinfo) == 2 and taskinfo[0] == FORTASK:
			taskId = int( taskinfo[1] )
		return taskId

	def pickFocusOrder ( curline ):
		curorder = []
		curline = curline.lstrip().rstrip().rstrip('\r\n')
		orderinfo = split ( curline[ curline.find(FORFBFOCUS) + len(FORFBFOCUS) + 1:] )
		if len(orderinfo) == 5:
			curorder = orderinfo
		return curorder

	sfh = file(srcfn,"r")
	if None == sfh:
		raise Exception, "Failed to open file - %s." % (srcfn)

	lnCnt = 0
	lasttime = 0
			
	''' read one task by another '''
	taskid = None
	curfbindices=[]
	fbidx=0

	participantInfo = dict()

	curline = sfh.readline()
	# overlook lines before the first TASK tag
	while string.find( curline, FORTASK ) == -1:
		lnCnt += 1
		curline = sfh.readline()

	rottimeorigin=0
	rotstarttime=0
	rotfinishtime=0
	rotlist=[]
	subrotlist=[]
	while curline:
		lnCnt += 1
		if string.find( curline, FORTASK ) != -1:
			taskid = pickTaskId( curline )
			if taskid == -1:
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)
			curline = sfh.readline()
			continue

		if string.find( curline, FORFBFOCUS ) != -1:
			curfbseq = pickFocusOrder( curline )
			#print "curfbseq: %s" % curfbseq
			curfbindices=[]
			for fb in curfbseq:
				curfbindices.append( FBNAMES[fb] )
			fbidx=0
			curline = sfh.readline()
			continue

		if string.find( curline, FORRESOLUTION ) != -1:
			resolution = pickResolution( curline )
			if resolution == (-1,-1,-1):
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)
			curline = sfh.readline()
			continue

		if string.find( curline, FORSTART ) != -1:
			lasttime = pickTime( curline )
			if lasttime == -1:
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)
			curline = sfh.readline()
			if rottimeorigin == 0:
				rottimeorigin = lasttime
			continue

		if string.find( curline, FORROTSTART ) != -1:
			rotstarttime = pickTime( curline )
			if rotstarttime == -1:
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)
			curline = sfh.readline()
			continue

		if string.find( curline, FORANGLE ) != -1:
			rottime = pickTime( curline )
			if rottime == -1:
				raise ValueError, "parsing %s failed at line No. %d - %s when retrieving time in rotation info line." % (srcfn,lnCnt,curline)

			rotinfo = pickRotInfo( curline )
			if len(rotinfo) < 4:
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)
			subrotlist.append( (rottime-rottimeorigin, rotinfo[0], rotinfo[1], rotinfo[2], rotinfo[3]) )
			curline = sfh.readline()
			continue

		if string.find( curline, FORROTEND ) != -1:
			rotfinishtime = pickTime( curline )
			if rotfinishtime == -1:
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)
			rotlist.append( (rotfinishtime - rotstarttime, subrotlist) )
			subrotlist=[]
			curline = sfh.readline()
			continue

		if string.find( curline, FORANSWER ) != -1:
			finishtime = pickTime( curline )
			if finishtime == -1:
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)
			singletasktime = finishtime - lasttime

			# now information for a single task collected, store it
			if taskid not in participantInfo.keys():
				participantInfo[ taskid ] = odict()
			if resolution not in participantInfo[ taskid ].keys():
				participantInfo[ taskid ][ resolution ] = list()
			participantInfo[ taskid ][ resolution ].append( (singletasktime, curfbindices[fbidx], rotlist) )
			rotlist=[]

			fbidx += 1
			curline = sfh.readline()
			continue

		print >> sys.stderr, "line skipped - %s" % curline
		curline = sfh.readline()
			
	sfh.close()
	print >> sys.stderr, "%d lines loaded in file %s" % (lnCnt, srcfn)
	global g_allparticipantinfo
	g_allparticipantinfo.append ( participantInfo )

## @brief read the run logs for all participants
## @param imfns the list of immediate files
def loadall(srcfns):
	pidx = 0
	#try:
	for srcfn in srcfns:
		loadsingle( srcfn )
		pidx += 1
	#except Exception, e:
		#print >> sys.stderr, "Failed to parse source files : %s" % (e)
	print >> sys.stderr, "%d participants have been analyzed." % (pidx)

## @brief serialize pariticipant info to the stdout file
def dump():
	global g_allparticipantinfo

	DataCategories=['CST','CG','CC','IFO','ILF']
	blockNum = 6 
	totalPt = 0
	totalLn = 0
	''' VTK legacy file header '''
	print >> sys.stdout, "# vtk DataFile Version 1.0"
	print >> sys.stdout, "User action in DTI density experiment"
	print >> sys.stdout, "ASCII"
	print >> sys.stdout
	print >> sys.stdout, "DATASET POLYDATA"
	for pidx in range( 0, len( g_allparticipantinfo ) ):
		for tid in g_allparticipantinfo[pidx].keys():
			for blockId in range(0, blockNum):
				for res in g_allparticipantinfo[pidx][tid].keys():
					if len (g_allparticipantinfo[pidx][tid][res]) < 1+blockId:
						continue

					ta=g_allparticipantinfo[pidx][tid][res][blockId]
					totalLn += 1
					for subrot in ta[2]:
						totalPt += len( subrot[1] )

	#print >> sys.stdout, "DATASET UNSTRUCTURED_GRID"
	print >> sys.stdout, "POINTS %d float" % (totalPt)
	for pidx in range( 0, len( g_allparticipantinfo ) ):
		for tid in g_allparticipantinfo[pidx].keys():
			for blockId in range(0, blockNum):
				for res in g_allparticipantinfo[pidx][tid].keys():
					if len (g_allparticipantinfo[pidx][tid][res]) < 1+blockId:
						continue

					ta=g_allparticipantinfo[pidx][tid][res][blockId]

					p0 = Point(0.0, 0.0, 1.0)
					for subrot in ta[2]:
						rotduration = float( subrot[0] )

						for rotel in subrot[1]:
							rottime = float( rotel[0] )
							rx, ry, rz, angle = float( rotel[1] ), float( rotel[2] ), float( rotel[3] ), float( rotel[4] )
							p1 = Point(0.0, 0.0, 0.0)
							p2 = Point().update((rx,ry,rz))
							theta = angle * pi / 180.0
							p0r = PointRotate3D(p1,p2,p0, theta)
							print >> sys.stdout, "%f %f %f" % ( p0r.x, p0r.y, p0r.z )
							p0 = p0r

	print >> sys.stdout
	print >> sys.stdout, "LINES %d %d %d" % (totalLn, totalPt+1, totalPt)
	for idx in range(0, totalPt):
		print >> sys.stdout, "%d" % idx
	print >> sys.stdout

	print >> sys.stdout, "POINT_DATA %d" % ( totalPt )
	print >> sys.stdout, "SCALARS timeTick float" 
	print >> sys.stdout, "LOOKUP_TABLE default" 
	for pidx in range( 0, len( g_allparticipantinfo ) ):
		for tid in g_allparticipantinfo[pidx].keys():
			for blockId in range(0, blockNum):
				for res in g_allparticipantinfo[pidx][tid].keys():
					if len (g_allparticipantinfo[pidx][tid][res]) < 1+blockId:
						continue

					ta=g_allparticipantinfo[pidx][tid][res][blockId]
					for subrot in ta[2]:
						rotduration = float( subrot[0] )

						for rotel in subrot[1]:
							rottime = float( rotel[0] )
							print >> sys.stdout, "%f" % ( rottime )

## @brief remove all immediate files
def removeImfiles():
	global g_allimfiles
	for imfn in g_allimfiles:
		os.system(" rm -rf %s " % (imfn) )

######################################
# the boost
if __name__ == "__main__":
	try:
		ParseCommandLine()
	except Exception,e:
		print >> sys.stderr, e
		sys.exit(1)

	preprocess( g_fns )
	loadall( g_allimfiles )
	dump()
	if not g_bKeepImfile:
		removeImfiles()
		
# set ts=4 tw=100 sts=4 sw=4
 
