#ifndef MEMORY_H
#define MEMORY_H

#include <systemc.h>
#include <iostream>

#include "memory_if.h"
#include "helpers.h"

class Memory : public memory_if, public sc_module {

public:
    SC_CTOR(Memory) 
    {
        // nothing to do here right now.
    }

    ~Memory() 
    {
        // nothing to do here right now.
    }

    int read(uint32_t addr);
    int write(uint32_t addr, int data);
};
#endif