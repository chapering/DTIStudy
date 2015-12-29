#!/usr/bin/env python
'''
Resize a DTI streamline model by scaling the voxel coordinates by given ratio
(indirectly by MaxCoord(ref2) / MaxCoord(ref1)

The result will be dumped to STDOUT, while error and prompt messages to STDERR

Copyright Chapstudio 2010-2011
Haipeng Cai @ June 21th, 2011
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

'''file containing coordinates needing flips, box position file in our case'''
g_fnsrc = None

''' two models providing the scaling ratio '''
g_fnref1 = None
g_fnref2 = None

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <src file> <ref1 file> <ref2 file> \n" % sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fnsrc, g_fnref1, g_fnref2
	argc = len(sys.argv)
	if argc >= 4:
		g_fnsrc = sys.argv[1]
		g_fnref1 = sys.argv[2]
		g_fnref2 = sys.argv[3]
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

	if not os.path.exists(g_fnsrc):
		raise IOError, "source file given [%s] does not exist, bailed out now." % g_fnsrc
	if not os.path.exists(g_fnref1):
		raise IOError, "source file given [%s] does not exist, bailed out now." % g_fnref1
	if not os.path.exists(g_fnref2):
		raise IOError, "source file given [%s] does not exist, bailed out now." % g_fnref2

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
	global g_fnsrc, g_fnref1, g_fnref2
	lcminmax1 = getMinMax( g_fnref1 )
	lcminmax2 = getMinMax( g_fnref2 )

	hfn = file (g_fnsrc, 'r')
	if None == hfn:
		raise Exception, "Failed to open file - %s." % (g_fnsrc)

	curline = "#"
	while curline:
		curline = hfn.readline()

		words = split ( curline )
		if len(words) != 6:
			print >>sys.stdout, curline,
			continue

		res = "%f %f %f" % ( 
				float(words[0]) * (lcminmax2[1][0] / lcminmax1[1][0]),
				float(words[1]) * (lcminmax2[1][1] / lcminmax1[1][1]), 
				float(words[2]) * (lcminmax2[1][2] / lcminmax1[1][2]) )

		print >> sys.stdout, res, ' '.join(words[3:6])

	hfn.close()
	
if __name__ == "__main__":
	try:
		ParseCommandLine()
		transform()
	except Exception,e:
		print >> sys.stderr, "%s" % (e)

# set ts=4 tw=80 sts=4 sw=4


