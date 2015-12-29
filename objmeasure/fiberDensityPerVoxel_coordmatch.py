#!/usr/bin/env python
'''
Purpose:
	In order to measure the uncertainty existing in the tractographies esp. those
	produced from lower seeding resolutions than the aquiescent 1x1x1.
Function:
	calculate Average Fiber Density per Voxel - count fibers passing through
	each non-empty voxel, then average the total amount by the number of
	non-empty voxels

	it also computes the "Tract volume" by accumulating the volume of voxels,
	taking the seeding resolution into account

The result will be dumped to STDOUT, while error and prompt messages to STDERR

Copyright Chapstudio 2010-2011
Haipeng Cai @ Sept 7th, 2011
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

'''seeding resolution'''
g_s = 1

''' storage of all unique voxels with their coordinates'''
g_allvoxels = list()

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <src file> <seeding resolution>\n" % sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fnsrc,g_s
	argc = len(sys.argv)
	if argc >= 3:
		g_fnsrc = sys.argv[1]
		g_s = int(sys.argv[2])
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

	if not os.path.exists(g_fnsrc):
		raise IOError, "source file given [%s] does not exist, bailed out now." % g_fnsrc

## @return True if equal and False otherwise
def compf(f1, f2): # compare two floating point numbers
	return math.fabs(f1 - f2) < 1e-3

## @return True if equal and False otherwise
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
		
## @brief compare two fiber tubes to decide if they are the same, regardless of
##	the order of fiber vertices
## @param fb1 list of the vertices in the 1st fiber tube
## @param fb2 list of the vertices in the 2nd fiber tube
## @return True or False indicating if these two fiber tubes are same or not
def compFiberTube(fb1, fb2):
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

## @return True if found and False otherwise
def findVoxel(voxel, store):
	for v in store:
		if compVertices( v, voxel ):
			return True
	return False

def findVoxelInTube(voxel, tubestore):
	for tube in tubestore:
		if findInTube(voxel, tube):
			return True
	return False

## @brief load all streamline geometries in the given file
## @param srcfn a string giving the name of file to parse
## @return a list of voxels
def loadfibers(srcfn):
	global g_allvoxels
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
	totalVoxel = 0
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
			voxel = (float(words[0]), float(words[1]), float(words[2]))
			singleLn.append ( voxel )
			if not findVoxel(voxel, g_allvoxels):
				g_allvoxels.append ( voxel )

			vtCnt += 1

		if vtCnt < vtTotal :
			raise IOError, "Error encountered at line %d, aborted.\n" % (lnCnt)

		totalVoxel += vtTotal
		lnCnt += 1
		print >> sys.stderr, "now in the No. %d line" % (lnCnt)
		allstlines.append(singleLn)
			
	sfh.close()
	print >> sys.stderr, "\t%d lines loaded from data file %s - totally %d non-empty voxels" % (lnCnt, srcfn, totalVoxel)
	return (allstlines,totalVoxel)

def calc():
	global g_fnsrc, g_allvoxels,g_s
	alllines,totalVoxel = loadfibers(g_fnsrc)

	voxelStat = dict()
	for voxel in g_allvoxels:
		if voxel not in voxelStat.keys():
			voxelStat[voxel] = 0

		if findVoxelInTube(voxel, alllines):
			voxelStat[voxel] += 1

	''' dump result and stat the target metrics'''
	totalCnt = 0
	for voxel,cnt in voxelStat.items():
		print >> sys.stderr, "(%f,%f,%f) %d" % ( voxel[0],voxel[1],voxel[2],cnt)
		totalCnt += cnt

	# dump results
	print >> sys.stdout, "average fiber density per voxel = %f " % \
			(totalCnt*1.0 / len(g_allvoxels))
	print >> sys.stdout, "Tract Volume = %f " % \
			(1.0*1.0*.997059*totalVoxel* (g_s**3))
	
if __name__ == "__main__":
	try:
		ParseCommandLine()
		calc()
	except Exception,e:
		print >> sys.stderr, "%s" % (e)

# set ts=4 tw=80 sts=4 sw=4

