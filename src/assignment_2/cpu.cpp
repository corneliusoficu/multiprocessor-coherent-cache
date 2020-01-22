#include "cpu.h"

CPU::CPU(sc_module_name name_, int id_) : sc_module(name_), id(id_)
{
    SC_THREAD(execute);
    sensitive << clock.pos();
    log(name(), "constructed with id", id);       
    dont_initialize();
}

void CPU::execute()
    {
        TraceFile::Entry tr_data;

        // Loop until end of tracefile
        while(!tracefile_ptr->eof())
        {
            // Get the next action for the processor in the trace
            if(!tracefile_ptr->next(0, tr_data))
            {
                cerr << "Error reading trace for CPU" << endl;
                break;
            }

            switch(tr_data.type)
            {
                case TraceFile::ENTRY_TYPE_READ:
                {
                    log(name(), "reading from address", tr_data.addr);
                    cache->cpu_read(tr_data.addr);
                    log(name(), "read done");
                    break;
                }
                case TraceFile::ENTRY_TYPE_WRITE:
                {
                    log(name(), "writing to address", tr_data.addr);
                    uint32_t data = rand();
                    cache->cpu_write(tr_data.addr, data);
                    log(name(), "write done");
                    break;
                }
                case TraceFile::ENTRY_TYPE_NOP:
                {
                    log(name(), "nop");
                    break;
                }
                default:
                {
                    cerr << "Error, got invalid data from Trace" << endl;
                    exit(0);
                }
            }
            wait();
        }

        // Finished the Tracefile, now stop the simulation
        sc_stop();
    }