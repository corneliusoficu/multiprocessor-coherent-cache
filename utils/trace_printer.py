#!/usr/bin/env python2

import subprocess
import struct
import sys

def read32(f): return f.read(4)

(TYPE_NOP, TYPE_READ, TYPE_WRITE, TYPE_END) = range(4)
e_types = ["NOP", "READ", "WRITE", "END"]

def entry(t, addr): write32(t | (addr & ~3))

# main
if len(sys.argv) < 2:
    print "usage: trace_printer <file>"
    exit(1)

f = open(sys.argv[1])
print read32(f)
nprocs = struct.unpack_from(">I", read32(f))[0]
print "nr of procs:", nprocs

while True:
    for p in range(nprocs):
        b = read32(f)
        if not b:
            break
        n = struct.unpack_from(">I", b)[0]
        t = n & 3
        addr = n & ~3
        print ("P%d" % p, e_types[t], addr) 
    if not b:
        break
f.close()
