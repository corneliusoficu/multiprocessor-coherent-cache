#include "cpu.h"

void CPU::execute()
    {
        TraceFile::Entry tr_data;
        Cache::Function  f;

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
                    f = Cache::FUNC_READ;
                    break;

                case TraceFile::ENTRY_TYPE_WRITE:
                    f = Cache::FUNC_WRITE;
                    break;

                case TraceFile::ENTRY_TYPE_NOP:
                    break;

                default:
                    cerr << "Error, got invalid data from Trace" << endl;
                    exit(0);
            }

            if(tr_data.type != TraceFile::ENTRY_TYPE_NOP)
            {
                Port_CacheAddr.write(tr_data.addr);
                Port_CacheFunc.write(f);

                if (f == Cache::FUNC_WRITE)
                {
                    cout << sc_time_stamp() << ": CPU sends write" << endl;

                    uint32_t data = rand();
                    Port_CacheData.write(data);
                    wait();
                    Port_CacheData.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
                }
                else
                {
                    cout << sc_time_stamp() << ": CPU sends read" << endl;
                }

                wait(Port_CacheDone.value_changed_event());

                if (f == Cache::FUNC_READ)
                {
                    cout << sc_time_stamp() << ": CPU reads: " << Port_CacheData.read() << endl;
                }
            }
            else
            {
                cout << sc_time_stamp() << ": CPU executes NOP" << endl;
            }
            // Advance one cycle in simulated time
            wait();
        }

        // Finished the Tracefile, now stop the simulation
        sc_stop();
    }