#ifndef BUS_H
#define BUS_H

#include <systemc.h>

class Bus_if : public virtual sc_interface
{
    public:
        virtual bool read(int addr) = 0;
        virtual bool write(int addr, int data) = 0;
};

class Bus : public Bus_if, public sc_module
{
    public:
        sc_in<bool>      Port_CLK;
        sc_signal_rv<32> Port_BusAddr;

        SC_CTOR(Bus)
        {
            // Handle Port_CLK to simulate delay
            // Initialize some bus properties
        }

        virtual bool read(int addr);
        virtual bool write(int addr, int data);
};

#endif