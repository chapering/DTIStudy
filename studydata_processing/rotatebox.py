#!/usr/bin/env python
'''
similar to flipstline.py, but used to flip coordinates in specified dimension of
a box whose positions were previously sampled in a brain region geometry that is
also needed to be flipped. Since the region would have been flipped, the box
should also be treated the same way to guarantee the relative box location to
the region to be still.

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
from PointRotate import *

split=string.split
getcontext().prec = 6
getcontext().rounding = ROUND_UP

'''file containing coordinates needing flips, box position file in our case'''
g_fnsrc = None

# direction along which the model will be flipped
# 1 for X, 2 for Y and 3 for Z
g_fcoord=None

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <src file> <coord to flip> \n" % sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fnsrc, g_fnregion, g_fcoord
	argc = len(sys.argv)
	if argc >= 3:
		g_fnsrc = sys.argv[1]
		g_fcoord = int(sys.argv[2])
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

	if not os.path.exists(g_fnsrc):
		raise IOError, "source file given [%s] does not exist, bailed out now." % g_fnsrc
	if g_fcoord not in range(1,4):
		raise IOError, "invalid argument [%d] for <coord to flip>" % g_fcoord

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
	global g_fnsrc, g_fcoord
	lcminmax = getMinMax( g_fnsrc )
	fcoord = g_fcoord

	hfn = file (g_fnsrc, 'r')
	if None == hfn:
		raise Exception, "Failed to open file - %s." % (g_fnsrc)

	ROTATE_AXES = {1:(1,0,0), 2:(0,1,0), 3:(0,0,1)}
	theta = 180.0 * math.pi / 180.0
	origin = Point()
	axis = Point().update(ROTATE_AXES[fcoord])
	offset = Point( 
			(lcminmax[0][0] + lcminmax[1][0])*.5,
			(lcminmax[0][1] + lcminmax[1][1])*.5,
			(lcminmax[0][2] + lcminmax[1][2])*.5)
		
	curline = "#"
	while curline:
		curline = hfn.readline()

		words = split ( curline )
		if len(words) != 3:
			print >>sys.stdout, curline,
			continue

		p0 = Point( float(words[0]), float(words[1]), float(words[2]) )
		# flip only the requested coordinate for each vertex
		res = PointRotate3D( origin, axis, p0, theta)

		print >> sys.stdout, res

	hfn.close()
	
if __name__ == "__main__":
	try:
		ParseCommandLine()
		transform()
	except Exception,e:
		print >> sys.stderr, "%s" % (e)

# set ts=4 tw=80 sts=4 sw=4


