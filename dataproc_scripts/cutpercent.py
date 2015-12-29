#!/usr/bin/env python
'''
cut given percent of fiber indices from the file specified and output the
indices that ought to be kept to the stdout

Copyright Chapstudio 2010-2011
Haipeng Cai @ June 17th, 2011
'''

import os
import sys
import string
import math
import random

def rset( r, p ):
	retset = set()
	total = len( r )
	cnt = 0
	while cnt < int ( math.ceil( total*p ) ):
		random.seed()
		curn = random.randint(0, total-1)
		if curn in retset:
			continue
		retset.add ( curn )
		cnt += 1
	
	return retset

def cutfromfile(fh, p):
	assert ( isinstance ( fh, file ) )
	# skip the first line
	fl = fh.readline()

	num = int ( fh.readline() )

	r = range(0, num)
	indices = rset ( r, p )

	icnt  = 0
	curs = fh.readline()

	print >> sys.stdout, "%s%d" % (fl, num - len(indices) )
	while curs:
		if  icnt not in indices:
			print >> sys.stdout, "%d" % ( int(curs) )

		icnt += 1
		curs = fh.readline()
	
if __name__ == "__main__":
	if len(sys.argv) < 2:
		print >> sys.stderr, "Usage: %s <source file> [cut percentage]" % ( sys.argv[0] )
		sys.exit(0)

	fn = sys.argv[1]
	if not os.path.isfile ( fn ):
		print >> sys.stderr, "Failed to access to file %s." % (fn)
		sys.exit(1)

	fh = file(fn, "r")
	if not fh:
		print >> sys.stderr, "Failed to open file %s." % (fn)
		sys.exit(2)

	percentage = 0.2
	if len(sys.argv) >= 3:
		percentage = float( sys.argv[2] )

	try:
		cutfromfile ( fh, percentage )
	finally:
		fh.close()
	

# set ts=4 tw=80 sts=4 sw=4

