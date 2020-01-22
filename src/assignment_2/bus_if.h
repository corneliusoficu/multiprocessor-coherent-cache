#include <systemc.h>

#ifndef BUS_IF_H
#define BUS_IF_H

class Bus_if : public virtual sc_interface
{
    public:
        virtual bool read(int addr) = 0;
        virtual bool write(int addr, int data) = 0;
};

#endif