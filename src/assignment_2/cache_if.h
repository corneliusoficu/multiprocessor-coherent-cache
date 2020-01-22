#include <systemc.h>

#ifndef CACHE_IF_H
#define CACHE_IF_H

class cache_if : public virtual sc_interface {
    public:
    virtual int cpu_read(uint32_t addr) = 0;
    virtual int cpu_write(uint32_t addr, u_int32_t data) = 0;
};

#endif