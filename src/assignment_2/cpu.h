#include <systemc.h>

#include "cache.h"

SC_MODULE(CPU)
{

public:
    sc_in<bool>             Port_CLK;
    sc_in<Cache::RetCode>   Port_CacheDone;
    sc_out<Cache::Function> Port_CacheFunc;
    sc_out<int>             Port_CacheAddr;
    sc_inout_rv<32>         Port_CacheData;

    SC_CTOR(CPU)
    {
        SC_THREAD(execute);
        sensitive << Port_CLK.pos();
        dont_initialize();
    }

private:
    void execute();
};