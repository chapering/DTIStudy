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

As provides general functionality of coordinate transfering, the tool is
dedicated to transform coordinate in the <src file> that was sampled in geometry 
from <1st data file> to adapt to the geometry in <2nd data file> in order to
keep the same relative object position in the second geometry.

The result will be dumped to STDOUT, while error and prompt messages to STDERR

Copyright Chapstudio 2010-2011
Haipeng Cai @ June 20th, 2011
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

'''
whole normal brain bounding size of all resolutions
normal_whole_brain_size = (
		({ 'X' : (51.248875, 182.808350),
		  'Y' : (37.959595, 204.101395),
		  'Z' : (5.032031, 134.905716) },
		289401)
		)
'''

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <src file> <1st data file> <2nd data file> \n" % sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fnsrc, g_fn1st, g_fn2nd
	argc = len(sys.argv)
	if argc >= 4:
		g_fnsrc = sys.argv[1]
		g_fn1st = sys.argv[2]
		g_fn2nd = sys.argv[3]
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

	if not os.path.exists(g_fnsrc):
		raise IOError, "source file given [%s] does not exist, bailed out now." % g_fnsrc
	if not os.path.exists(g_fn1st):
		raise IOError, "1st geometry file given [%s] does not exist, bailed out now." % g_fn1st
	if not os.path.exists(g_fn2nd):
		raise IOError, "2nd geometry file given [%s] does not exist, bailed out now." % g_fn2nd

## @brief retrieve the bounding box of the geometry in the given file
def getMinMax(fn):
	hfn = file (fn, 'r')
	if None == hfn:
		raise Exception, "Failed to open file - %s." % (fn)

	lcmin = [ sys.maxint, sys.maxint, sys.maxint ]
	lcmax = [ -sys.maxint, -sys.maxint, -sys.maxint ]

	print >> sys.stderr, "retrieving bounding box of geometry in %s ..." % (fn)

	curline = "#"
	while curline:
		curline = hfn.readline()

		words = split ( curline )
		if len(words) < 3:
			continue

		for i in range(0,3):
			fw = float(words[i])
			if fw > lcmax[i]:
				lcmax[i] = fw
			if fw < lcmin[i]:
				lcmin[i] = fw

	hfn.close()

	print >> sys.stderr, "\tMax : %s \n\tMin: %s\n" % (
			','.join(`n` for n in lcmin ), 
			','.join(`n` for n in lcmax ) )

	return [lcmin, lcmax]

def transform():
	global g_fnsrc, g_fn1st, g_fn2nd
	lcminmax1 = getMinMax( g_fn1st )
	lcminmax2 = getMinMax( g_fn2nd )

	offsets = [.0, .0, .0]
	for i in range(0,3):
		offsets[i] = (lcminmax1[0][i] + lcminmax1[1][i]) * .5 - \
				(lcminmax2[0][i] + lcminmax2[1][i]) * .5;

	hfn = file (g_fnsrc, 'r')
	if None == hfn:
		raise Exception, "Failed to open file - %s." % (g_fnsrc)
		
	curline = "#"
	while curline:
		curline = hfn.readline()

		words = split ( curline )
		if len(words) != 3:
			print >>sys.stdout, curline,
			continue

		coord = [0, 0, 0]
		for i in range(0,3):
			coord[i] = float ( words[i] ) + offsets[i]	

		print >> sys.stdout, ' '.join(`n` for n in coord)

	hfn.close()
	
if __name__ == "__main__":
	try:
		ParseCommandLine()
		transform()
	except Exception,e:
		print >> sys.stderr, "%s" % (e)

# set ts=4 tw=80 sts=4 sw=4


