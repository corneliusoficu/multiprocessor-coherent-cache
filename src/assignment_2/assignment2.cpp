#include <systemc>
#include <iostream>
#include <stdlib.h>

#include "cpu.h"
#include "cache.h"

using namespace std;
using namespace sc_core; // This pollutes namespace, better: only import what you need.

int sc_main(int argc, char* argv[])
{
    try
    {
        // Get the tracefile argument and create Tracefile object
        // This function sets tracefile_ptr and num_cpus
        init_tracefile(&argc, &argv);

        if (argc == 2 && !strcmp(argv[0], "-q")) 
        {
            sc_report_handler::set_verbosity_level(SC_LOW);
        }

        sc_set_time_resolution(1, SC_PS);

        // Initialize statistics counters
        stats_init();

        CPU   *cpu   = new CPU("cpu", 0);
        Cache *cache = new Cache("cache", 0); 

        // The clock that will drive the CPU
        sc_clock clk;

        cpu->cache(*cache);
        cpu->clock(clk);

        // Start Simulation
        sc_start();

        // Print statistics after simulation finished
        stats_print();
        cout << sc_time_stamp() <<endl;

        delete cpu;
        delete cache;
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }

    return 0;
}
