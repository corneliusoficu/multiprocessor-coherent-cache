#include "cache.h"


Cache::Cache(sc_module_name name_, int id_) : sc_module(name_), id(id_)
{
    initialize_cache_arrays();
    SC_THREAD(snoop);
    sensitive << port_clk.pos();
    dont_initialize();
}

Cache::~Cache() 
{
    for(int index = 0; index < CACHE_NUMBER_OF_SETS; index++)
    {
        delete[] least_recently_updated[index];
        delete[] cache[index];
        delete[] tags[index];
    }

    delete[] least_recently_updated;
    delete[] cache;
    delete[] tags;   
}

void Cache::initialize_cache_arrays()
{
    cache                   = new int*[CACHE_NUMBER_OF_SETS];
    tags                    = new int*[CACHE_NUMBER_OF_SETS]; 
    least_recently_updated  = new int*[CACHE_NUMBER_OF_SETS];

    for(int i = 0; i < CACHE_NUMBER_OF_SETS; i++)
    {
        least_recently_updated[i] = new int[CACHE_NUMBER_OF_LINES_IN_SET];
        cache[i]                  = new int[CACHE_NUMBER_OF_LINES_IN_SET * CACHE_LINE_SIZE_BYTES];
        tags[i]                   = new int[CACHE_NUMBER_OF_LINES_IN_SET];

        for(int j = 0; j < CACHE_NUMBER_OF_LINES_IN_SET; j++)
        {
            tags[i][j] = -1;
        }
    }
}

int Cache::create_mask(int start_bit, int end_bit)
{
    int mask = 0;
    for(int i  = start_bit; i <= end_bit; i++)
    {
        mask |= 1 << i;
    }
    return mask;
}

int Cache::get_index_of_line_in_set(int set_index, int tag)
{
    for(int index = 0; index < CACHE_NUMBER_OF_LINES_IN_SET; index++)
    {
        if(tags[set_index][index] == tag)
        {
            return index;
        }
    }
    return -1;
}

void Cache::update_lru(int set_address, int line_in_set_index)
{
    for(int i = 0; i < CACHE_NUMBER_OF_LINES_IN_SET; i++)
    {
        if(least_recently_updated[set_address][i] < least_recently_updated[set_address][line_in_set_index])
        {
            least_recently_updated[set_address][i]++;
        }
    }
    least_recently_updated[set_address][line_in_set_index] = 0;
}

int Cache::get_lru_line(int set_address){ //returns index of the lru line
    int max = 0;
    int max_index = 0;

    for(int i = 0; i < CACHE_NUMBER_OF_LINES_IN_SET; i++)
    {
        if(least_recently_updated[set_address][i] > max)
        {
            max =  least_recently_updated[set_address][i];
            max_index = i;
        }

    }
    return max_index;
}

void Cache::extract_address_components(u_int32_t addr, int *byte_in_line, int *set_address, int *tag)
{
    *byte_in_line = (addr & bit_mask_byte_in_line);     // Obtaining value for bits 0 - 4, no shifting required
    *set_address  = (addr & bit_mask_set_address) >> 5; // Shifting to right to obtain value for bits 5  - 11
    *tag          = (addr & bit_mask_tag) >> 12;  
}

int Cache::cpu_read(uint32_t addr)
{
    int byte_in_line, set_address, tag, locked_proc_id_mutex;
    extract_address_components(addr, &byte_in_line, &set_address, &tag);

    int line_in_set_index = get_index_of_line_in_set(set_address, tag);
            
    if(line_in_set_index == -1)
    {
        log(name(), "read miss on address", addr);
        stats_readmiss(id);
        
        locked_proc_id_mutex = bus->check_ongoing_requests(id, addr, BusRequest::READ);
        bus->read(id, addr);
        
        line_in_set_index = get_lru_line(set_address);
        tags[set_address][line_in_set_index] = tag;
        cache[set_address][line_in_set_index * CACHE_LINE_SIZE_BYTES + byte_in_line] = rand() % 1000 + 1;
        
        bus->release_mutex(id, addr);
        if(locked_proc_id_mutex != -1)
        {
            bus->release_mutex(locked_proc_id_mutex, addr);
        }
        wait();
    }
    else
    {
        log(name(), "read hit on address", addr);
        update_lru(set_address, line_in_set_index);
        stats_readhit(id);
        wait();
    }
    
    update_lru(set_address, line_in_set_index);
    return 0;
}

int Cache::cpu_write(uint32_t addr, uint32_t data)
{
    int byte_in_line, set_address, tag, locked_proc_id_mutex;
    extract_address_components(addr, &byte_in_line, &set_address, &tag);

    int line_in_set_index = get_index_of_line_in_set(set_address, tag);

    if(line_in_set_index == -1)
    {
        log(name(), "write miss on address", addr);
        stats_writemiss(id);

        line_in_set_index = get_lru_line(set_address);
        tags[set_address][line_in_set_index] = tag;
        wait();
    }
    else
    {
        log(name(), "write hit on address", addr);
        stats_writehit(id);

        locked_proc_id_mutex = bus->check_ongoing_requests(id, addr, BusRequest::WRITE);
        bus->write(id, addr, data);
        
        update_lru(set_address, line_in_set_index);

        bus->release_mutex(id, addr);
        if(locked_proc_id_mutex != -1)
        {
            bus->release_mutex(locked_proc_id_mutex, addr);
        }
        wait();
    }

    cache[set_address][line_in_set_index * CACHE_NUMBER_OF_LINES_IN_SET] = data;

    return 0;
}

void Cache::invalidate_cache_copy(int addr)
{

}

void Cache::snoop()
{
    while(can_snoop)
    {
        wait(port_bus_valid.value_changed_event());
        
        int snooped_address     = port_bus_addr.read().to_int();
        BusRequest request_type = port_bus_valid.read();
        int proc_index          = port_bus_proc.read();

        handle_snooped_value(snooped_address, request_type, proc_index); 
    }
}

void Cache::handle_snooped_value(int snooped_address, BusRequest request_type, int proc_index)
{
    switch(request_type)
    {   
        case WRITE:
        case READX:
            if(proc_index != id)
            {
                invalidate_cache_copy(snooped_address);
            }
            break;
        case READ:
        case INVALID:
            break;
    }
}