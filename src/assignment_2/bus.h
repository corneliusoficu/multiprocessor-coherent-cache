#ifndef BUS_H
#define BUS_H

#include <systemc.h>

#include "bus_if.h"
#include "cache.h"

class Bus : public Bus_if, public sc_module
{
    public:
        enum Request
        {
            INVALID,
            READ,
            WRITE,
            READX
        };

        typedef struct
        {
            sc_mutex access_mutex;
            int      address;
            Request  request_type;
        } bus_request;
        
        sc_signal_rv<32>   Port_BusAddr;
        sc_out<Cache::Req> Port_BusValid;
        sc_out<int>        Port_BusProc;

        Bus(sc_module_name, int, int);
        ~Bus();

        virtual bool read(int, int);
        virtual bool write(int, int, int);
    
    private:
        int id;
        int num_cpus;
        bus_request *requests;
        sc_mutex bus_mutex;
};

#endif