#include <systemc.h>

#ifndef MEMORY_IF_H
#define MEMORY_IF_H

class memory_if : public virtual sc_interface {
    public:
    virtual int read(uint32_t addr) = 0;
    virtual int write(uint32_t addr, int data) = 0;
};

#endif