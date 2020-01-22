#ifndef CPU_H
#define CPU_H

#include <systemc.h>
#include <iostream>

#include "cache_if.h"
#include "helpers.h"
#include "psa.h"

class CPU : public sc_module
{

public:
    sc_in_clk         clock;
    sc_port<cache_if> cache;

    CPU(sc_module_name, int);

    SC_HAS_PROCESS(CPU);

private:
    int id;

    void execute();
};

#endif