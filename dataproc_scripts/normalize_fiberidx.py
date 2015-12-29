#!/usr/bin/env python
'''
now that I have already been tormenting for a month to produce a plethora of
task data for the long dragging project, the DTI seeding resolution/fiber
density study, they were built in three dimension (partial brain to load, fiber
bundle focus, task-specific parameters like box positions, fiber indices and so
on), where the later 3rd dimensiond depends on the partial region selected as
the data to be loaded in a specific task. 

Now after the first pilot study which gave unideal result (only show
significant difference in time by resolutions in Task 2, and at most in Task 1
and Task 3 and 6, but not significant in other Tasks), more blocks of data has
been suggested to use. The first one is to use whole brain or complete half
brain as the initially loaded data consistently across all tasks. However, the
3rd dimension should be kept as that in the task dataset I already made (named
the block 2, since the block 3 will only use mere focused fiber bundles, there
appears a linear downscaling of contextual fibers from block 1 to 3)

So the problem is to adapt the 3rd dimension parameters in block 2 to that for
the block 1 and 3 to be made.

As provides general functionality of fiber index transfering, the tool is
dedicated to transform fiber indices in the <src file> that was sampled in geometry 
from <1st data file> to adapt to the geometry in <2nd data file> in order to
keep the same fiber tubes to be indiced in the second geometry.

The result will be dumped to STDOUT, while error and prompt messages to STDERR

Copyright Chapstudio 2010-2011
Haipeng Cai @ July 21th, 2011
		Add extra output in conjugation with the number of fiber indices if
		there is any stuff in original fiber index file (to keep the
		compatibility with the task data for Task 6 in the density study) 
		@July 22th
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

'''file containing coordinates needing calibration, box position file in our case'''
g_fnsrc = None

'''file of the first geometry data, typically that of the partial brain geometry'''
g_fn1st = None

'''file of the second geometry, typically the whole brain geometry containing the partial brain data'''
g_fn2nd = None

'''optional argument, indicating how many lines in the head of g_fnsrc should be
skipped while parsing and transforming fiber indices
'''
g_ndiscard = 0

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <src file> <1st data file> <2nd data file>" \
							" [#line to discard]\n" % sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fnsrc, g_fn1st, g_fn2nd, g_ndiscard
	argc = len(sys.argv)
	if argc >= 4:
		g_fnsrc = sys.argv[1]
		g_fn1st = sys.argv[2]
		g_fn2nd = sys.argv[3]
		if argc >= 5:
			g_ndiscard = int(sys.argv[4])
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

	if not os.path.exists(g_fnsrc):
		raise IOError, "source file given [%s] does not exist, bailed out now." % g_fnsrc
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

	lnTotal = int(curline)
	lnCnt = 0
	while curline and lnCnt < lnTotal:
		# read line by line, in order to avoid the memory swelling by otherwise loading all lines once
		curline = sfh.readline()
		curline = curline.lstrip().rstrip('\r\n')
		if len(curline) < 1 or curline in ["#","//","/*","%"]:
			continue

		vtTotal = int(curline)
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
			singleLn.append( (float(words[0]), float(words[1]), float(words[2])) )

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
	global g_fnsrc, g_fn1st, g_fn2nd, g_ndiscard
	allline1 = loadfibers(g_fn1st)
	allline2 = loadfibers(g_fn2nd)

	hfn = file (g_fnsrc, 'r')
	if None == hfn:
		raise Exception, "Failed to open file - %s." % (g_fnsrc)
		
	ndiscard = g_ndiscard
	nline = 0
	''' discard lines that are not fiber indices '''
	curline = "#"
	while curline and nline < ndiscard:
		curline = hfn.readline()
		nline += 1

	if nline < ndiscard:
		raise ValueError, "FATAL: reach to the end before discarding all lines requested."

	orgnum = 0
	''' this line is expected to tell the number of indices following'''
	curline = hfn.readline()
	words = split( curline )
	orgnum = int( words[0] )

	def findTube(fb, fbstore):
		for i in range(0, len(fbstore)):
			if compFiberTube(fb, fbstore[i]):
				return i
		return -1

	newfiberindices = list()
	curidxcnt = 0
	while curline and curidxcnt < orgnum:
		curline = hfn.readline()
		fidx = int(curline)

		if fidx < 0 or fidx >= len(allline1):
			raise ValueError, "FATAL: fiber index out of range with respect to " \
				"data file given - %s." % ( g_fn1st )

		newidx = findTube( allline1[ fidx ], allline2 )

		if -1 == newidx:
			print >> sys.stderr, "fiber of index  %d in %s can not be found in " \
				"%s --> dropped." % (fidx, g_fn1st, g_fn2nd)
		else:
			newfiberindices.append( newidx )

		curidxcnt += 1

	hfn.close()

	# dump results
	if len(words) >= 2:
		print >> sys.stdout, "%s %s" % ( len(newfiberindices), ' '.join(words[1:]) )
	else:
		print >> sys.stdout, len(newfiberindices)
	for idx in newfiberindices:
		print >> sys.stdout, idx

	if len(newfiberindices) < orgnum:
		print >> sys.stderr, "Warning: only %d of %d fibers have been" \
				" translated." % ( len(newfiberindices), orgnum )
	
if __name__ == "__main__":
	try:
		ParseCommandLine()
		transform()
	except Exception,e:
		print >> sys.stderr, "%s" % (e)

# set ts=4 tw=80 sts=4 sw=4


