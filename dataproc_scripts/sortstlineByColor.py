#!/usr/bin/env python
'''
read a DTI streamline model and class them into fiber bundles
according to vertex color, the basic prerequisite is that:
all vertice in a streamline are assigned with constant color 

Copyright Chapstudio 2010-2011
Haipeng Cai @ May 8, 2011
'''

import os
import sys
import string
import re

# name of file to parse
g_fn=None

# precision of color similarity
g_precision=1
g_cutThreshold=3
g_bRoundoff=True

# hole all streamlines
g_allstlines=[]
g_color2lines={}

split=string.split

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <src file> <precision> [CutThreshold] [rounding off]\n" % \
			sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fn, g_precision
	global g_cutThreshold,g_bRoundoff
	argc = len(sys.argv)
	if argc >= 2:
		g_fn = sys.argv[1]
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

	if argc >= 3:
		g_precision = int(sys.argv[2])
	if argc >= 4:
		g_cutThreshold = int(sys.argv[3])
	if argc >= 5:
		g_bRoundoff = ( int(sys.argv[4]) != 0 )

	if not os.path.exists(g_fn):
		raise IOError, "source file given [%s] does not exist, bailed out now." % g_fn
	if g_precision not in range(1,7):
		raise IOError, "invalid argument [%d] for <coord to flip>" % g_precision

## @brief load all streamline geometries and sort into bundles according to
##	 color similarity
## @param srcfn a string giving the name of file to parse
## @param precision a number in [1,6] giving the clustering precision
def loadAndSort(srcfn,precision):
	global g_allstlines, g_color2lines
	global g_bRoundoff

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
		r,g,b = 1,1,1
		while curline and vtCnt < vtTotal:
			curline = sfh.readline()
			curline = curline.lstrip().rstrip('\r\n')
			if len(curline) < 1 or curline in ["#","//","/*","%"]:
				continue

			# alway splitting a line with whitespace as the delimiter
			words = split(curline)
			if len(words) < 6: # not a vertex line
				continue

			singleLn.append( curline )
		
			extra = 0
			if g_bRoundoff:
				extra = 5.0/pow(10,precision+1)
			if vtCnt == 0: # all vertices have the same color as expected
				fmt = "%%.%df" % precision
				r = fmt % (float(words[3])+extra)
				g = fmt % (float(words[4])+extra)
				b = fmt % (float(words[5])+extra)

			vtCnt += 1

		if vtCnt < vtTotal :
			raise IOError, "Error encountered at line %d, aborted.\n" % (lnCnt)

		if (r,g,b) not in g_color2lines.keys():
			g_color2lines[(r,g,b)] = list()
			'''
			print "new bundle with color : %s,%s,%s" % (r,g,b)
			'''
		g_color2lines[(r,g,b)].append( lnCnt )

		g_allstlines.append(singleLn)

		lnCnt += 1
			
	sfh.close()
	print >> sys.stderr, "%d lines loaded, initially %d bundles sorted out" % \
			(lnCnt, len(g_color2lines))

## @brief serialize bundles into separate files, following the format of the
##  input
def dump():
	global g_allstlines, g_color2lines
	global g_cutThreshold

	bundleCnt = 0
	trbundleCnt = 0
	for color,lnIds in g_color2lines.items():
		lnNum = len(lnIds)
		# should we ignore the very trivial bundles
		if lnNum < g_cutThreshold:
			trbundleCnt += 1
			continue

		bundleCnt += 1
		bundlefn = "bundle%d" % bundleCnt
		bfh = file(bundlefn, 'w')
		bfh.write("%d\r\n" % lnNum)

		for lid in lnIds:
			vtNum = len( g_allstlines[ lid ] )

			bfh.write("%d\r\n" % vtNum)
			for vid in range(0, vtNum):
				bfh.write("%s\r\n" % g_allstlines[ lid ][ vid ])

		bfh.close()
		print >> sys.stderr, "bundle No.%d dumped into file %s." % \
				(bundleCnt, bundlefn)

	if trbundleCnt > 0:
		print >> sys.stderr, "\nFinally %d bundles ignored due to containing less than %d fibers." % \
				(trbundleCnt, g_cutThreshold)

######################################
# the boost
if __name__ == "__main__":
	try:
		ParseCommandLine()
		print >> sys.stderr, "Loading and sorting......"
		loadAndSort(g_fn, g_precision)
		dump()
		policy = "was"
		if not g_bRoundoff:
			policy = "not"
		print >> sys.stderr, "Rounding off %s adopted during the color" \
					"similarity based sorting.\n" % (policy)
	except Exception,e:
		print >> sys.stderr, e
		
# set ts=4 tw=100 sts=4 sw=4

