#include "bus.h"

bool Bus::read(int addr)
{
    // Bus might be in contention
    Port_BusAddr.write(addr);
    return true;
}   

bool Bus::write(int addr, int data)
{
    // Handle contention if any
    Port_BusAddr.write(addr);
    // Data does not have to be handled in the simulation
    return true;
}