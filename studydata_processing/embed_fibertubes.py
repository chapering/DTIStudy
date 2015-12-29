#!/usr/bin/env python
'''
embed fiber tract attributes in <2nd src>, all items after coordinates, into corresponding 
fiber in <1st src> by matching fiber vertex coordinate

The result will be dumped to STDOUT, while error and prompt messages to STDERR

Copyright Chapstudio 2010-2011
Haipeng Cai @ Aug 14th, 2011
'''

import os
import sys
import string
import math
import random
from decimal import *
split=string.split
getcontext().prec = 6
getcontext().rounding = ROUND_UP

'''file of the first geometry data'''
g_fn1st = None

'''file of the second geometry'''
g_fn2nd = None

'''extra data to be embedded for each fiber '''
g_extradata = ""

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <1st data file> <2nd data file>" \
							" [extra data to be embedded]\n" % sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fn1st, g_fn2nd, g_extradata
	argc = len(sys.argv)
	if argc >= 3:
		g_fn1st = sys.argv[1]
		g_fn2nd = sys.argv[2]
		if argc >= 4:
			g_extradata = int(sys.argv[3])
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

	if not os.path.exists(g_fn1st):
		raise IOError, "1st geometry file given [%s] does not exist, bailed out now." % g_fn1st
	if not os.path.exists(g_fn2nd):
		raise IOError, "2nd geometry file given [%s] does not exist, bailed out now." % g_fn2nd

## @brief load all streamline geometries in the given file
## @param srcfn a string giving the name of file to parse
## @return a list of fiber tubes
def loadfibers(srcfn):
	allstlines = [] # [ [(x,y,z),(x,y,z),...], [(x,y,z),(x,y,z),...], ... ]

	sfh = file(srcfn,"r")
	if None == sfh:
		raise Exception, "Failed to open file - %s." % (srcfn)

	curline = "#"
	while curline:
		curline = sfh.readline()
		curline = curline.lstrip().rstrip('\r\n')
		if len(curline) >=1 and curline not in ["#","//","/*","%"]:
			break

	lnTotal = int(split(curline)[0])
	lnCnt = 0
	while curline and lnCnt < lnTotal:
		# read line by line, in order to avoid the memory swelling by otherwise loading all lines once
		curline = sfh.readline()
		curline = curline.lstrip().rstrip('\r\n')
		if len(curline) < 1 or curline in ["#","//","/*","%"]:
			continue

		vtTotal = int(split(curline)[0])
		vtCnt = 0

		singleLn = list()
		while curline and vtCnt < vtTotal:
			curline = sfh.readline()
			curline = curline.lstrip().rstrip('\r\n')
			if len(curline) < 1 or curline in ["#","//","/*","%"]:
				continue

			# alway splitting a line with whitespace as the delimiter
			words = split(curline)
			if len(words) < 6: # not a vertex line
				continue

			''' in this tool, color, if exists, is what we do not care about 
				the first three numbers are presumed to indicate the 3D
				coordinates of fiber vertices
			'''
			singleLn.append( (float(words[0]), float(words[1]), float(words[2]), \
							float(words[3]), float(words[4]), float(words[5])) )

			vtCnt += 1

		if vtCnt < vtTotal :
			raise IOError, "Error encountered at line %d, aborted.\n" % (lnCnt)

		allstlines.append(singleLn)

		lnCnt += 1
			
	sfh.close()
	print >> sys.stderr, "\t%d lines loaded from data file %s" % (lnCnt, srcfn)
	return allstlines

## @brief compare two fiber tubes to decide if they are the same, regardless of
##	the order of fiber vertices
## @param fb1 list of the vertices in the 1st fiber tube
## @param fb2 list of the vertices in the 2nd fiber tube
## @return True or False indicating if these two fiber tubes are same or not
def compFiberTube(fb1, fb2):
	def compf(f1, f2): # compare two floating point numbers
		return math.fabs(f1 - f2) < 1e-3

	def compVertices(v1, v2): # compare two vertices
		for i in range(0,3):
			if not compf( v1[i], v2[i] ):
				return False
		return True

	def findInTube(v, fb):
		for i in range(0, len(fb)):
			if compVertices(v, fb[i]):
				return i
		return -1
		
	if len(fb1) != len(fb2):
		return False

	for i in range(0, len(fb1)):
		if -1 == findInTube(fb1[i], fb2):
			return False
	
	return True

def transform():
	global g_fn1st, g_fn2nd, g_extradata
	allline1 = loadfibers(g_fn1st)
	allline2 = loadfibers(g_fn2nd)

	def findTube(fb, fbstore):
		for i in range(0, len(fbstore)):
			if compFiberTube(fb, fbstore[i]):
				return i
		return -1

	idxcnt = 0
	orgnum = len(allline2)

	print >> sys.stdout, len(allline1)
	for line in allline1:
		idx = findTube( line, allline2 )

		if -1 == idx:
			print >> sys.stdout, "%d 0" % (len(line))
			for vert in line:
				for d in vert:
					print >> sys.stdout, "%f" % d,
				print
		else:
			print >> sys.stdout, "%d %s" % (len(line), g_extradata)
			for vert in allline2[idx]:
				for d in vert:
					print >> sys.stdout, "%f" % d,
				print
			idxcnt += 1
			
	if idxcnt < orgnum:
		print >> sys.stderr, "Warning: only %d of %d fibers in %s have been" \
				" embedded into %s." % ( idxcnt, orgnum, g_fn2nd, g_fn1st )
	
if __name__ == "__main__":
	try:
		ParseCommandLine()
		transform()
	except Exception,e:
		print >> sys.stderr, "%s" % (e)

# set ts=4 tw=80 sts=4 sw=4


