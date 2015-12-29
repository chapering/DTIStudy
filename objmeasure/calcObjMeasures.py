#!/usr/bin/env python
'''
Purpose:
	In order to measure the uncertainty existing in the tractographies esp. those
	produced from lower seeding resolutions than the aquiescent 1x1x1.
Function:
	calculate three core metrics of the nine parameters proposed by Stephen
	Correia used for measuring white matter integrity:
		. NS - number of streamtubes in a TOI
		. TL - Total length of streamtubes in a TOI in minimeter
		. TWLcl - Total tube length in a TOI weighted by linear anisotropy
	and
		. Average FA - the FA average of all the voxels in the TOI streamtubes

The result will be dumped to STDOUT, while error and prompt messages to STDERR

Copyright Chapstudio 2010-2011
Haipeng Cai @ Sept 6th, 2011
'''

import os
import sys
import string
import math
import random
from decimal import *
split=string.split
sqrt=math.sqrt
getcontext().prec = 6
getcontext().rounding = ROUND_UP

'''file containing coordinates needing metrics calculation, might it be a WHOLE
brain or a TOI'''
g_fnsrc = None

'''file of the integral tensor information '''
g_fntensorinfo = None

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <src file> <tensor data file>\n" % sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fnsrc, g_fntensorinfo
	argc = len(sys.argv)
	if argc >= 3:
		g_fnsrc = sys.argv[1]
		g_fntensorinfo = sys.argv[2]
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

	if not os.path.exists(g_fnsrc):
		raise IOError, "source file given [%s] does not exist, bailed out now." % g_fnsrc
	if not os.path.exists(g_fntensorinfo):
		raise IOError, "1st geometry file given [%s] does not exist, bailed out now." % g_fntensorinfo

## @brief load all streamline geometries in the given file and find tensor 
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
			singleLn.append( (float(words[0]), float(words[1]), float(words[2]),\
							float(words[3]), \
							float(words[4]), float(words[5]), float(words[6]) )
							)

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

def findTube(fb, fbstore):
	for i in range(0, len(fbstore)):
		if compFiberTube(fb, fbstore[i]):
			return i
	return -1

def calc():
	global g_fnsrc, g_fntensorinfo
	alltensorinfo = loadfibers(g_fntensorinfo)

	hfn = file (g_fnsrc, 'r')
	if None == hfn:
		raise Exception, "Failed to open file - %s." % (g_fnsrc)
		
	orgnum = 0
	''' this line is expected to tell the number of indices following'''
	curline = hfn.readline()
	words = split( curline )
	orgnum = int( words[0] )

	curidxcnt = 0
	totalLen = .0
	totalFA = .0
	totalVoxel = 0
	TWLcl = .0
	TWLfa = .0
	while curline and curidxcnt < orgnum:
		curline = hfn.readline()

		curline = curline.lstrip().rstrip('\r\n')
		if len(curline) < 1 or curline in ["#","//","/*","%"]:
			continue

		vtTotal = int(curline)
		vtCnt = 0

		singleLn = list()
		prewords = None
		segLen = .0
		while curline and vtCnt < vtTotal:
			curline = hfn.readline()
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
			words = (float(words[0]), float(words[1]), float(words[2]))
			singleLn.append( words )
			if prewords :
				segLen += sqrt( (words[0]-prewords[0])**2 + 
						(words[1]-prewords[1])**2 + 
						(words[2]-prewords[2])**2 )
			prewords = words

			vtCnt += 1

		if vtCnt < vtTotal :
			raise IOError, "Error encountered at line %d, aborted.\n" % (lnCnt)

		totalLen += segLen

		newidx = findTube( singleLn, alltensorinfo )

		if -1 == newidx:
			print >> sys.stderr, "FATAL: the No.%d fiber tube in %s can not be found in " \
				"%s - aborted." % (curidxcnt+1, g_fnsrc, g_fntensorinfo)
			return

		''' calculate linear anisotropy and FA '''
		sumCL = .0
		sumFA = .0
		for tensor in alltensorinfo[newidx]: # go through all voxels in this tube
			x,y,z = float( tensor[0] ), float( tensor[1] ), float ( tensor[2] )
			l1, l2, l3 = float( tensor[4] ), float( tensor[5] ), float ( tensor[6] )
			fa = float ( tensor[3] )
			cl = ( l1 - l2 ) / (l1 + l2 + l3)
			fa_calc = sqrt( (l1 - l2)**2 + (l2 - l3)**2 + (l1 - l3)**2 ) / \
					sqrt (l1**2 + l2**2 + l3**2) * sqrt (1.0/2.0) 

			totalFA += fa_calc
			totalVoxel += 1
			sumCL += cl
			sumFA += fa_calc
			
		TWLcl += (segLen * 1.0 * ( sumCL * 1.0 / len ( alltensorinfo[newidx] )))
		TWLfa += (segLen * 1.0 * ( sumFA * 1.0 / len ( alltensorinfo[newidx] )))

		curidxcnt += 1

	hfn.close()

	if curidxcnt < orgnum:
		print >> sys.stderr, "Error: only %d of %d fibers read into..." % (curidxcnt, orgnum)
		return

	# dump results
	print >> sys.stderr, "============================================"
	print >> sys.stdout, "NS\tTL\tTWLcl\tTWLfa\tATL\tAvg FA"
	print >> sys.stdout, "%d\t%f\t%f\t%f\t%f\t%f" % ( orgnum, totalLen, TWLcl, \
			TWLfa, totalLen/orgnum, totalFA * 1.0 / totalVoxel )
	
if __name__ == "__main__":
	try:
		ParseCommandLine()
		calc()
	except Exception,e:
		print >> sys.stderr, "%s" % (e)

# set ts=4 tw=80 sts=4 sw=4

