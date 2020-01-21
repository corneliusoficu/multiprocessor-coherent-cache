/*
 * File: assignment1.cpp
 *
 * Framework to implement Task 1 of the Advances in Computer Architecture lab
 * session. This uses the framework library to interface with tracefiles which
 * will drive the read/write requests
 *
 * Author(s): Michiel W. van Tol, Mike Lankamp, Jony Zhang,
 *            Konstantinos Bousias
 * Copyright (C) 2005-2017 by Computer Systems Architecture group,
 *                            University of Amsterdam
 *
 */

#include <systemc>
#include <iostream>
#include <stdlib.h>

#include "cpu.h"

using namespace std;
using namespace sc_core; // This pollutes namespace, better: only import what you need.

int sc_main(int argc, char* argv[])
{
    try
    {
        // Get the tracefile argument and create Tracefile object
        // This function sets tracefile_ptr and num_cpus
        init_tracefile(&argc, &argv);

        // Initialize statistics counters
        stats_init();

        // Instantiate Modules
        Cache cache("L1 Cache");
        CPU   cpu("cpu");

        // Signals
        sc_buffer<Cache::Function> sigCacheFunc;
        sc_buffer<Cache::RetCode>  sigCacheDone;
        sc_signal<int>             sigCacheAddr;
        sc_signal_rv<32>           sigCacheData;

        // The clock that will drive the CPU and Cache
        sc_clock clk;

        // Connecting module ports with signals
        cache.Port_Func(sigCacheFunc);
        cache.Port_Addr(sigCacheAddr);
        cache.Port_Data(sigCacheData);
        cache.Port_Done(sigCacheDone);

        cpu.Port_CacheFunc(sigCacheFunc);
        cpu.Port_CacheAddr(sigCacheAddr);
        cpu.Port_CacheData(sigCacheData);
        cpu.Port_CacheDone(sigCacheDone);

        cache.Port_CLK(clk);
        cpu.Port_CLK(clk);

        cout << "Running (press CTRL+C to interrupt)... " << endl;


        // Start Simulation
        sc_start();

        // Print statistics after simulation finished
        stats_print();
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }

    return 0;
}
