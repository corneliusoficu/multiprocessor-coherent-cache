#include <systemc>
#include <iostream>
#include <stdlib.h>

#include "cpu.h"
#include "cache.h"
#include "memory.h"

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

void init_cpus_and_caches(CPU** cpus, Cache** caches, int nr_cpus, Memory* memory, sc_clock *clk)
{   
    char name_cpu[20];
    char name_cache[20];

    Cache* cache;
    CPU*   cpu;

    for(int index = 0; index < nr_cpus; index++)
    {
        sprintf(name_cpu,   "cpu_%d",  index);
        sprintf(name_cache, "cache_%d", index);

        cpu   = new CPU(name_cpu, index);
        cache = new Cache(name_cache, index);

        cpu->cache(*cache);
        cpu->clock(*clk);
        cache->memory(*memory);

        cpus[index]   = cpu;
        caches[index] = cache;
    }
}


int sc_main(int argc, char* argv[])
{
    try
    {
        init_tracefile(&argc, &argv);
        int nr_processors = num_cpus;

        if (argc == 2 && !strcmp(argv[0], "-q")) 
        {
            sc_report_handler::set_verbosity_level(SC_LOW);
        }

        sc_set_time_resolution(1, SC_PS);

        stats_init();

        CPU*    cpus[num_cpus];
        Cache*  caches[num_cpus];
        Memory* memory = new Memory("memory");

        sc_clock clk;

        init_cpus_and_caches(cpus, caches, nr_processors, memory, &clk);

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
