#ifndef BUS_H
#define BUS_H

#include <systemc.h>

#include "bus_if.h"
#include "memory_if.h"
#include "cache.h"

class Bus : public Bus_if, public sc_module
{
    public:
        long waits;
        long reads;
        long writes;
        long readXs;
        long consistency_waits;

        sc_in<bool>        port_clk;
        sc_port<memory_if> memory;        
        sc_signal_rv<32>   port_bus_addr;
        sc_out<BusRequest> port_bus_valid;
        sc_out<int>        port_bus_proc;

        Bus(sc_module_name, int, int);
        ~Bus();

        virtual bool read(int, int);
        virtual bool write(int, int, int);
        virtual bool readx(int, int, int);

        virtual int  check_ongoing_requests(int, int, BusRequest);
        virtual void release_mutex(int, int);
    
    private:
        int id;
        int num_cpus;

        RequestContent *requests;
        sc_mutex bus_mutex;
};

#endif