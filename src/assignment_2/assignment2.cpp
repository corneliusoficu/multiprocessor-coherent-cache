#include <systemc>
#include <iostream>
#include <stdlib.h>

#include "cpu.h"
#include "cache.h"
#include "memory.h"
#include "bus.h"

using namespace std;
using namespace sc_core; // This pollutes namespace, better: only import what you need.

void delete_cpus_and_caches(CPU **cpus, Cache** caches, int nr_cpus)
{
    for(int index = 0; index < nr_cpus; index++)
    {
        delete cpus[index];
        delete caches[index];
    }
}

void init_bus_cpus_and_caches(Bus *bus, CPU** cpus, 
                              Cache** caches, int nr_cpus, 
                              Memory* memory, sc_clock *clk, 
                              sc_signal<int> *sig_bus_proc,
                              sc_signal<BusRequest> *sig_bus_valid)
{   
    char name_cpu[20], name_cache[20];

    Cache* cache;
    CPU*   cpu;

    bus->memory(*memory);
    bus->port_clk(*clk);
    bus->port_bus_proc(*sig_bus_proc);
    bus->port_bus_valid(*sig_bus_valid);

    for(int index = 0; index < nr_cpus; index++)
    {
        sprintf(name_cpu,   "cpu_%d",   index);
        sprintf(name_cache, "cache_%d", index);

        cpu   = new CPU  (name_cpu,   index);
        cache = new Cache(name_cache, index);

        cpu->cache(*cache);
        cpu->clock(*clk);

        cache->bus(*bus);
        cache->port_bus_addr(bus->port_bus_addr);
        cache->port_bus_proc(*sig_bus_proc);
        cache->port_bus_valid(*sig_bus_valid);
        cache->can_snoop = true;
        cache->port_clk(*clk);

        cpus[index]   = cpu;
        caches[index] = cache;
    }
}

int sc_main(int argc, char* argv[])
{
    try
    {
        init_tracefile(&argc, &argv);

        sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);
        sc_report_handler::set_actions(SC_ID_LOGIC_X_TO_BOOL_,             SC_LOG);
        sc_report_handler::set_actions(SC_ID_VECTOR_CONTAINS_LOGIC_VALUE_, SC_LOG);

        int nr_processors = num_cpus;

        if (argc == 2 && !strcmp(argv[0], "-q")) 
        {
            sc_report_handler::set_verbosity_level(SC_LOW);
        }

        sc_set_time_resolution(1, SC_PS);

        stats_init();

        CPU*    cpus[num_cpus];
        Cache*  caches[num_cpus];
        
        Bus*    bus    = new Bus("bus", 0, nr_processors);
        Memory* memory = new Memory("memory");

        sc_clock              clk;
        sc_signal<int>        sig_bus_proc;
        sc_signal<BusRequest> sig_bus_valid;

        init_bus_cpus_and_caches(bus, cpus, caches, nr_processors, memory, &clk, &sig_bus_proc, &sig_bus_valid);

        sc_start();

        stats_print();
        cout << sc_time_stamp() <<endl;

        delete_cpus_and_caches(cpus, caches, nr_processors);
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }

    return 0;
}
