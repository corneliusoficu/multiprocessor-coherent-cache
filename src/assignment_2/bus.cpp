#include "bus.h"

Bus::Bus(sc_module_name name_, int id_, int num_cpus_) : sc_module(name_), id(id_), num_cpus(num_cpus_)
{
    sensitive << port_clk.pos();

    requests = new RequestContent[num_cpus];
    for(int cpu_index = 0; cpu_index < num_cpus; cpu_index++)
    {
        requests[cpu_index].address      = -1;
        requests[cpu_index].request_type = BusRequest::INVALID;
    }
    port_bus_addr.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");

    waits             = 0;
    reads             = 0;
    writes            = 0;
    readXs            = 0;
    consistency_waits = 0;
}

Bus::~Bus()
{
    delete[] requests;
}

bool Bus::read(int proc_index, int addr)
{
    while(requests[proc_index].access_mutex.trylock() == -1)
    {
        consistency_waits++;
        wait();
    }

    requests[proc_index].address      = addr;
    requests[proc_index].request_type = BusRequest::READ;


    while(bus_mutex.trylock() == -1)
    {
        waits++;
        wait();
    }

    reads++;

    log(name(), "read for address", addr, "proc index", proc_index);

    port_bus_addr.write(addr);
    port_bus_proc.write(proc_index);
    port_bus_valid.write(BusRequest::READ);

    wait();

    port_bus_valid.write(BusRequest::INVALID);
    port_bus_addr.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");

    bus_mutex.unlock();

    memory->read(addr);

    return true;
}   

bool Bus::write(int proc_index, int addr, int data)
{
    while(requests[proc_index].access_mutex.trylock() == -1)
    {
        consistency_waits++;
        wait();
    }

    requests[proc_index].address      = addr;
    requests[proc_index].request_type = BusRequest::WRITE;

    while(bus_mutex.trylock() == -1)
    {
        wait();
        waits++;
    }

    writes++;

    log(name(), "write for address", addr, "proc index", proc_index);

    port_bus_addr.write(addr);
    port_bus_proc.write(proc_index);
    port_bus_valid.write(BusRequest::WRITE);

    wait();

    port_bus_valid.write(BusRequest::INVALID);
    port_bus_addr.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
    
    bus_mutex.unlock();

    memory->write(addr, data);

    return true;
}

bool Bus::readx(int proc_index, int addr, int data)
{
    while(requests[proc_index].access_mutex.trylock() == -1)
    {
        consistency_waits++;
        wait();
    }

    requests[proc_index].address = addr;
    requests[proc_index].request_type = BusRequest::READX;

    while(bus_mutex.trylock() == -1)
    {
        waits++;
        wait();
    }

    readXs++;

    log(name(), "readx for address", addr, "proc index", proc_index);
    
    port_bus_addr.write(addr);
    port_bus_proc.write(proc_index);
    port_bus_valid.write(BusRequest::READX);

    wait();

    port_bus_valid.write(BusRequest::INVALID);
    port_bus_addr.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");

    bus_mutex.unlock();

    memory->read(addr);
    memory->write(addr, data);

    return true;
}

void Bus::release_mutex(int proc_index, int addr)
{
    if(requests[proc_index].address == addr)
    {
        requests[proc_index].access_mutex.unlock();
        requests[proc_index].address      = -1;
        requests[proc_index].request_type = BusRequest::INVALID;
    }
}

int Bus::check_ongoing_requests(int proc_index, int addr, BusRequest operation)
{
    bool acquired_lock = false;
    BusRequest pending_operation;
    int p_index;

    for(p_index = 0; p_index < num_cpus; p_index++)
    {
        if(requests[p_index].address == addr)
        {
            pending_operation = requests[p_index].request_type;
            
            if((pending_operation == READ && (operation == READX || operation == WRITE)) || 
               (pending_operation == READX && (operation == READX || operation == READ)))
            {
                while(requests[p_index].access_mutex.trylock() == -1)
                {
                    consistency_waits++;
                    wait();
                }
                requests[p_index].address = addr;
                acquired_lock = true;
                break;
            }
        }
    }

    if(acquired_lock == false)
    {
        p_index = -1;
    }

    return p_index;
}