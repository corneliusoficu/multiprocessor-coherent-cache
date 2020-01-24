#include "bus.h"

Bus::Bus(sc_module_name name_, int id_, int num_cpus_) : sc_module(name_), id(id_), num_cpus(num_cpus_)
{
    requests = new bus_request[num_cpus];
    for(int cpu_index = 0; cpu_index < num_cpus; cpu_index++)
    {
        requests[cpu_index].address      = -1;
        requests[cpu_index].request_type = INVALID;
    }
    Port_BusAddr.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
}

Bus::~Bus()
{
    delete[] requests;
}

bool Bus::read(int proc_index, int addr)
{
    while(requests[proc_index].access_mutex.trylock() == -1)
    {
        wait();
    }

    requests[proc_index].address      = addr;
    requests[proc_index].request_type = READ;

    while(bus_mutex.trylock() == -1)
    {
        wait();
    }

    Port_BusAddr.write(addr);
    Port_BusProc.write(proc_index);
    Port_BusValid.write(Cache::READ);

    wait();

    Port_BusValid.write(Cache::INVALID);
    Port_BusAddr.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");

    bus_mutex.unlock();

    return true;
}   

bool Bus::write(int proc_index, int addr, int data)
{
    // Handle contention if any
    Port_BusAddr.write(addr);
    // Data does not have to be handled in the simulation
    return true;
}