#!/usr/bin/env python

import subprocess
import struct
def write32(n): f.write(struct.pack('>I', n))

def openTrace():
	global f, filename
	global rmiss, rhit, wmiss, whit
	(rmiss, rhit, wmiss, whit) = (0, 0, 0, 0)
	filename = "tmp.trf"
	f = open(filename, "w")
	f.write("2TRF") # trace file signature
	write32(1) # 1 processor

def closeTrace():
	f.close()
	p = subprocess.Popen(["./assignment_1.bin", filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	(stdoutdata, stderrdata) = p.communicate()
	if "Error" in stderrdata:
		print stderrdata
		return
	r = [int(i) for i in stdoutdata.split("\n")[-2].split()[:-1]]
	if r[2] != rhit or r[3] != rmiss or r[5] != whit or r[6] != wmiss: print "Fail: " + str(r)
	else: print "OK"

(TYPE_NOP, TYPE_READ, TYPE_WRITE, TYPE_END) = range(4)
def entry(t, addr): write32(t | (addr & ~3))

openTrace()

# our lines are 32/0x20 wide, 32kB of 8-way set-associative cache --> 128 sets
# fill set 0 (not present)
wmiss = wmiss + 9
for i in range(9): entry(TYPE_WRITE, i * 0x20 * 128)

entry(TYPE_END, 0)
closeTrace()

