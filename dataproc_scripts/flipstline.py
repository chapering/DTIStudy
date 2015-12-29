#!/usr/bin/env python
'''
Flip a DTI streamline model along the given direction.
The flipped model data will be dumped to the stdout

Copyright Chapstudio 2010-2011
Haipeng Cai @ May 8, 2011
'''

import os
import sys
import string
import re

# name of file to parse
g_fn=None

# direction along which the model will be flipped
# 1 for X, 2 for Y and 3 for Z
g_fcoord=None

g_maxcoord=-0xfffffffff
g_mincoord=0xfffffffff

split=string.split

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <src file> <coord to flip>\n" % sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fn, g_fcoord
	argc = len(sys.argv)
	if argc >= 3:
		g_fn = sys.argv[1]
		g_fcoord = int(sys.argv[2])
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

	if not os.path.exists(g_fn):
		raise IOError, "source file given [%s] does not exist, bailed out now." % g_fn
	if g_fcoord not in range(1,4):
		raise IOError, "invalid argument [%d] for <coord to flip>" % g_fcoord


## @brief find the maximum and minimum of the flipping coordinates
## @param srcfn a string giving the name of file to parse
## @param fcoord a number giving the direction along which to flip
def findMaxMin(srcfn, fcoord):
	global g_maxcoord, g_mincoord

	sfh = file(srcfn,"r")
	if None == sfh:
		raise Exception, "Failed to open file - %s." % (srcfn)

	curline = "#"
	while curline:
		# read line by line, in order to avoid the memory swelling by otherwise loading all lines once
		curline = sfh.readline()
		'''terseline = curline.lstrip().rstrip('\r\n')'''
			
		# alway splitting a line with whitespace as the delimiter
		words = split(curline)
		if len(words) < 6: # not a vertex line
			continue

		c = float( words[ fcoord - 1 ] )
		if c > g_maxcoord :
			g_maxcoord = c
		if c < g_mincoord :
			g_mincoord = c
		
	sfh.close()


## @brief parsing the input file and flip the model along the designated direction
## @param srcfn a string giving the name of file to parse
## @param fcoord a number giving the direction along which to flip
def flip(srcfn, fcoord):
	global g_maxcoord, g_mincoord

	sfh = file(srcfn,"r")
	if None == sfh:
		raise Exception, "Failed to open file - %s." % (srcfn)

	curline = "#"
	while curline:
		# read line by line, in order to avoid the memory swelling by otherwise loading all lines once
		curline = sfh.readline()
		curline = curline.lstrip().rstrip('\r\n')
			
		# alway splitting a line with whitespace as the delimiter
		words = split(curline)
		if len(words) < 6: # not a vertex line
			# write as it was
			print >> sys.stdout, curline
			continue

		# flip only the requested coordinate for each vertex
		trans = float( words[ fcoord - 1 ] ) - (g_maxcoord + g_mincoord)/2.0
		words[ fcoord - 1 ] = "%.6f" % ( -trans + (g_maxcoord + g_mincoord)/2.0 )
		
		print >> sys.stdout, " ".join(words)
		
	sfh.close()

######################################
# the boost
if __name__ == "__main__":
	try:
		ParseCommandLine()
		findMaxMin(g_fn, g_fcoord)
		print >> sys.stderr, "max: %f, min: %f" % (g_maxcoord, g_mincoord)
		flip(g_fn, g_fcoord)
	except Exception,e:
		print >> sys.stderr, e
		
# set ts=4 tw=100 sts=4 sw=4

