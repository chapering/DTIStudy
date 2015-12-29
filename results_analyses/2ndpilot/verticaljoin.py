#!/usr/bin/env python
'''
concatenate line by line from two files and print the horizontally joined lines
to the STDOUT

Haipeng Cai @ Aug 4th, 2011
'''

import os
import sys
import string

'''file containing coordinates needing calibration, box position file in our case'''
g_fnsrc1 = None

'''file of the first geometry data, typically that of the partial brain geometry'''
g_fnsrc2 = None

## @brief print usage
## @param none
## @retrun none
def Usage():
	print >> sys.stdout, "%s <src file 1> <src file 2>" % sys.argv[0]
	return

## @brief parse command line to get user's input for source file name and target
## @param none
## @retrun none
def ParseCommandLine():
	global g_fnsrc1, g_fnsrc2
	argc = len(sys.argv)
	if argc >= 3:
		g_fnsrc1 = sys.argv[1]
		g_fnsrc2 = sys.argv[2]
	else:
		Usage()
		raise Exception, "too few arguments, aborted."

	if not os.path.exists(g_fnsrc1):
		raise IOError, "1st source file given [%s] does not exist, bailed out now." % g_fnsrc1
	if not os.path.exists(g_fnsrc2):
		raise IOError, "2nd source file given [%s] does not exist, bailed out now." % g_fnsrc2

def horjoin():
	global g_fnsrc1, g_fnsrc2
	hf1 = file(g_fnsrc1, 'r')
	hf2 = file(g_fnsrc2, 'r')

	al1 = hf1.readlines()
	al2 = hf2.readlines()
	nline = min( len(al1), len(al2) )

	i=0
	while i < nline:
		print >>sys.stdout, "%s %s" % \
			( al1[i].rstrip("\r\n"), al2[i].rstrip("\r\n") )
		i += 1

	hf1.close()
	hf2.close()
	
if __name__ == "__main__":
	try:
		ParseCommandLine()
		horjoin()
	except Exception,e:
		print >> sys.stderr, "%s" % (e)

# set ts=4 tw=80 sts=4 sw=4


