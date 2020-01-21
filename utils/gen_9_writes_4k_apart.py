#!/usr/bin/env python2

import struct

(TYPE_NOP, TYPE_READ, TYPE_WRITE, TYPE_END) = range(4)

def write32(n): f.write(struct.pack('>I', n))

def entry(t, addr): write32(t | (addr & ~3))

def openTrace(filename, num_proc):
	global f
	f = open(filename, "w")
	f.write("2TRF") # trace file signature
	write32(num_proc)

openTrace("generated.trf", 1)

for i in range(9): entry(TYPE_WRITE, i * 0x20 * 128)

f.close()

