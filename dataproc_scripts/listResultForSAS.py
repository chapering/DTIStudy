#!/usr/bin/env python
'''
read a run log output of current seeding-study program and retrieve the most
important feature data like task completion time and the answer correctness.

after processing, the output will dump to the stdout

Copyright Chapstudio 2010-2011
Haipeng Cai @July 12th, 2011
	Add click distance to lesion center for lesion task @ Jul. 13th 2011
'''

import os
import sys
import string
import re
from odict import *

# name of file to parse
g_fns=[]

# hole all streamlines
g_allimfiles=[]
g_bKeepImfile=False

'''data structure:
	[ {taskid:{res:[(singletasktime, task answer),...], ...}, ...}, ... ]
'''
g_allparticipantinfo=[]

split=string.split

PREPROCESS_FILTER="TASK|Task completed|resolution|Time cost|Distance of click to lesion center"
# keywords for retrieving feature lines
FORTASK="TASK"
FORRESOLUTION="under seeding resolution of"
FORANSWER="Task completed with Answer"
FORCLICKDIST="Distance of click to lesion center"

# refer to array defintions in "the function task6() in runstudy_5p.sh"
CUTSCHEME=((2,2,0,1,2), (1,2,0,2,2), (1,2,1,2,0), (0,2,2,1,0), (1,0,2,0,0),
		(1,2,2,2,0), (0,0,0,1,1), (1,2,0,0,2), (0,1,1,2,0), (2,2,1,0,2))

# will load these two external reference if source file can be found in the
# current directory
g_allFADiffs=[]
g_allBundlesizeDiffs=[]

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

	for i in range(1,10):
		if os.path.isfile("./faDiff_block%d" % i):
			hfFA=file("./faDiff_block%d" % i,'r')
			g_allFADiffs.append(hfFA.readlines())
			hfFA.close()
		else:
			break

	for i in range(1,10):
		if os.path.isfile("./bundleDiffs_block%d" % i):
			hfbd=file("./bundleDiffs_block%d" % i)
			g_allBundlesizeDiffs.append(hfbd.readlines())
			hfbd.close()
		else:
			break

## @brief preprocess the raw log and filter out only relevant information 
##	for the statistics
## @param srcfns a list of raw log files
## @return none
def preprocess(srcfns):
	global g_allimfiles
	for rfn in srcfns:
		imfn = "%s_stat" % rfn
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
			timeinfo = split( timeinfo[0], ':', 2 )
			lasttime = int( timeinfo[0] ) * 3600 + int( timeinfo[1] ) * 60 + int (timeinfo[2])
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

	def pickAnswer( curline ) :
		answer = -1
		curline = curline.lstrip().rstrip(' \r\n')
		if len (curline) > 10 and string.find( curline, FORANSWER) != -1:
			ansinfo = split ( curline )
			if len(ansinfo) == 8 :
				# answer not available
				return -2
			tanswer = ansinfo[-1]
			if tanswer == "(correct).":
				answer = 1
			elif tanswer == "(wrong).":
				answer = 0
		return answer

	def pickClickDistance( curline ):
		dist = -2.0
		curline = curline.lstrip().rstrip(' \r\n')
		if len (curline) > 20 and string.find( curline, FORCLICKDIST) != -1:
			distinfo = split ( curline )
			if len(distinfo) == 10:
				dist = float( distinfo[-1] )
		return dist

	def pickTaskId( curline ):
		taskId = -1
		curline = curline.lstrip().rstrip().rstrip('\r\n')
		taskinfo = split( curline )
		if len(taskinfo) == 2 and taskinfo[0] == FORTASK:
			taskId = int( taskinfo[1] )
		return taskId

	sfh = file(srcfn,"r")
	if None == sfh:
		raise Exception, "Failed to open file - %s." % (srcfn)

	lnCnt = 0
	''' read the start time '''
	curline = "#"
	lasttime = 0
	while  curline:
		curline = sfh.readline()
		lnCnt += 1
		lasttime = pickTime( curline )
		if lasttime > 0 :
			break
			
	''' read one task by another '''
	taskid = None
	clkdist=-2.0

	participantInfo = dict()

	curline = sfh.readline()
	while curline:
		lnCnt += 1
		if string.find( curline, FORTASK ) != -1:
			taskid = pickTaskId( curline )
			if taskid == -1:
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)
			curline = sfh.readline()
			continue

		if string.find( curline, FORRESOLUTION ) != -1:
			resolution = pickResolution( curline )
			if resolution == (-1,-1,-1):
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)
			curline = sfh.readline()
			continue

		if string.find( curline, FORCLICKDIST ) != -1:
			clkdist = pickClickDistance( curline )
			curline = sfh.readline()
			continue

		if string.find( curline, FORANSWER ) != -1:
			finishtime = pickTime( curline )
			if finishtime == -1:
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)
			singletasktime = finishtime - lasttime
			lasttime = finishtime

			answer = pickAnswer( curline )
			if answer == -1:
				raise ValueError, "parsing %s failed at line No. %d - %s." % (srcfn,lnCnt,curline)

			# now information for a single task collected, store it
			if taskid not in participantInfo.keys():
				participantInfo[ taskid ] = odict()
			if resolution not in participantInfo[ taskid ].keys():
				participantInfo[ taskid ][ resolution ] = list()
			if clkdist >= 0.0:
				participantInfo[ taskid ][ resolution ].append( (singletasktime, answer, clkdist ) )
				clkdist=-2.0
			else:
				participantInfo[ taskid ][ resolution ].append( (singletasktime, answer ) )

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
	global g_allFADiffs
	global g_allBundlesizeDiffs

	''' this is title line '''
	DataCategories=['CST','CG','CC','IFO','ILF']
	blockNum = 2 #len ( g_allparticipantinfo[pidx][tid][res] )
	## print >> sys.stdout, "Pid\tTid\tTorder\tRes\tDid\tTime\tCorrect\tDprop"
	print >> sys.stdout, "Pid Tid Torder Res Did Time Correct Dprop"
	for pidx in range(0, len( g_allparticipantinfo ) ):
		tnum=1
		for tid in g_allparticipantinfo[pidx].keys():
			for blockId in range(0, blockNum):
				bfid=1
				for res in g_allparticipantinfo[pidx][tid].keys():
					did=bfid
					if tid == 6:
						if bfid == 3:
							did=5
						elif bfid == 4:
							did=1
						if did > 3:
							did=3
					if len (g_allparticipantinfo[pidx][tid][res]) < 1+blockId:
						continue
					ta=g_allparticipantinfo[pidx][tid][res][blockId]
					dprop=0
					if tid == 1 and len(g_allFADiffs)>blockId: # FA difference
						dprop=float( g_allFADiffs[blockId][ (did-1)*5+int(res[0])-1 ] )
					elif tid == 6 and len(g_allBundlesizeDiffs)>blockId: # bundle size difference
						dprop=float(
								g_allBundlesizeDiffs[blockId][(did-1)*3*5+(int(res[0])-1)*3 +  
									CUTSCHEME[ blockId*5 + pidx ][ bfid-1 ]])
					elif (tid == 4 or tid==7 ) and len(ta)>=3:
						dprop=float( ta[2] )

					print >> sys.stdout, "%d %d %d %s %s %d %d %f" % \
						(pidx+1, tid, tnum, res[0], did, ta[0], ta[1], dprop)
					tnum+=1
					bfid+=1

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
 
