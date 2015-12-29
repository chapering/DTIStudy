#!/usr/bin/env python
'''
The result will be dumped to STDOUT, while error and prompt messages to STDERR

Copyright Chapstudio 2010-2011
Haipeng Cai @Sept. 13rd, 2011
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

BASE=os.path.abspath(".")

g_allfbnum = dict()
def getavg():
	global g_allfbnum
	for cls in ("normal_whole", "normal_allfb", "normal_partial"):
		for res in range(1,6):
			fbtotal = .0
			itemnum = 0
			for _tid in range(1,7):
				tid = _tid
				if _tid == 6:
					tid = 4
				for fb in ("cst", "cg", "cc", "ifo", "ilf"):
					if cls=="normal_whole" and fb!="cc":
						continue
					for pos in ("", "pos1"):
						curfile="%s/%s/region_for_task%d/s%d/%s/%s/region_s%d.data" % \
								(BASE, cls, tid, res, fb, pos, res)
						if not os.path.isfile(curfile):
							raise IOError, "FATAL: %s not FOUND!" % (curfile)

						hfn = file(curfile,'r')
						fbnum = int( hfn.readline() )
						hfn.close()

						fbtotal += fbnum
						itemnum += 1

			if cls not in g_allfbnum.keys():
				g_allfbnum[cls] = dict()

			if res not in g_allfbnum[cls].keys():
				g_allfbnum[cls][res] = [.0,.0,.0]

			g_allfbnum[cls][res][0] = fbtotal*1.0/itemnum

def getstd():
	global g_allfbnum
	for cls in ("normal_whole", "normal_allfb", "normal_partial"):
		for res in range(1,6):
			fbtotal = .0
			itemnum = 0
			for _tid in range(1,7):
				tid = _tid
				if _tid == 6:
					tid = 4
				for fb in ("cst", "cg", "cc", "ifo", "ilf"):
					if cls=="normal_whole" and fb!="cc":
						continue
					for pos in ("", "pos1"):
						curfile="%s/%s/region_for_task%d/s%d/%s/%s/region_s%d.data" % \
								(BASE, cls, tid, res, fb, pos, res)
						if not os.path.isfile(curfile):
							raise IOError, "FATAL: %s not FOUND!" % (curfile)

						hfn = file(curfile,'r')
						fbnum = int( hfn.readline() )
						hfn.close()

						fbtotal += (fbnum - g_allfbnum[cls][res][0])**2
						itemnum += 1

			g_allfbnum[cls][res][1] = math.sqrt( fbtotal*1.0/itemnum )
			g_allfbnum[cls][res][2] = math.sqrt( fbtotal*1.0/itemnum ) / math.sqrt( itemnum*1.0 )

def dump():
	global g_allfbnum
	#print >> sys.stdout, "data\t\ts1\t\ts2\t\ts3\t\ts4\t\ts5"
	print >> sys.stdout, "data\t\t\tres\t\tavg\t\tstd\t\tstderr"
	print >> sys.stdout, "------------------------------------------",
	print >> sys.stdout, "------------------------------------------"
	for cls in g_allfbnum.keys():
		for res in g_allfbnum[cls].keys():
			print >> sys.stdout, "%s\t\ts%d\t\t" % (cls,res),
			print >> sys.stdout, "%d\t\t%d\t\t%d" % \
					(g_allfbnum[cls][res][0], g_allfbnum[cls][res][1], g_allfbnum[cls][res][2])
		
if __name__ == "__main__":
	try:
		getavg()
		getstd()
		dump()
	except Exception,e:
		print >> sys.stderr, "%s" % (e)

# set ts=4 tw=80 sts=4 sw=4


