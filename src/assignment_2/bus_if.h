#include <systemc.h>

#include "datatypes.h"

#ifndef BUS_IF_H
#define BUS_IF_H

class Bus_if : public virtual sc_interface
{
    public:
        virtual bool read(int proc_index, int addr) = 0;
        virtual bool write(int proc_index, int addr, int data) = 0;
        virtual bool readx(int proc_index, int addr, int data) = 0;
        
        virtual void release_mutex(int proc_index, int addr) = 0;
        virtual int  check_ongoing_requests(int proc_index, int addr, BusRequest operation) = 0;
};

#endif