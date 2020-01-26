#include "cache.h"

ofstream Log;

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
        delete[] cache[index];
        delete[] tags[index];
    }

    delete[] lru;
    delete[] cache;
    delete[] tags;   
}

void Cache::initialize_cache_arrays()
{
    cache                   = new int*[CACHE_NUMBER_OF_SETS];
    tags                    = new int*[CACHE_NUMBER_OF_SETS]; 
    cache_status            = new int*[CACHE_NUMBER_OF_SETS];
    lru                     = new u_int8_t[CACHE_NUMBER_OF_SETS];

    for(int i = 0; i < CACHE_NUMBER_OF_SETS; i++)
    {
        cache[i]                  = new int[CACHE_NUMBER_OF_LINES_IN_SET * CACHE_LINE_SIZE_BYTES];
        tags[i]                   = new int[CACHE_NUMBER_OF_LINES_IN_SET];
        cache_status[i]           = new int[CACHE_NUMBER_OF_LINES_IN_SET];

        for(int j = 0; j < CACHE_NUMBER_OF_LINES_IN_SET; j++)
        {
            tags[i][j]         = -1;
            cache_status[i][j] =  1; 
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
        if(cache_status[set_index][index] == 1 && tags[set_index][index] == tag)
        {
            return index;
        }
    }
    return -1;
}

void Cache::update_lru(int set_address, int line_in_set_index)
{
    switch(line_in_set_index)
    {
        case 0:
            lru[set_address] = lru[set_address] | 11;
            break;
        case 1:
            lru[set_address] = lru[set_address] | 3;
            lru[set_address] = lru[set_address] & 119;
            break;
        case 2:
            lru[set_address] = lru[set_address] | 17;
            lru[set_address] = lru[set_address] & 125;
            break;
        case 3:
            lru[set_address] = lru[set_address] | 1;
            lru[set_address] = lru[set_address] & 109;
            break;
        case 4:
            lru[set_address] = lru[set_address] | 36;
            lru[set_address] = lru[set_address] & 126;
            break;                   
        case 5:
            lru[set_address] = lru[set_address] | 4;
            lru[set_address] = lru[set_address] & 94;
            break;
        case 6:
            lru[set_address] = lru[set_address] | 64;
            lru[set_address] = lru[set_address] & 122;
            break;
        case 7:
            lru[set_address] = lru[set_address] & 58;
            break;
        default:
            break;
    }
}

int Cache::get_lru_line(int set_address)
{ 
    for(int i = 0; i < CACHE_NUMBER_OF_LINES_IN_SET; i++)
    {
        if(cache_status[set_address][i] == 0)
        {
            return i;
        }
    }

    u_int8_t lru_val = lru[set_address];

    if( (lru_val & 11) == 0 )
    {
        return 0;
    }
    else if( (lru_val & 11) == 8 )
    {
        return 1;
    }
    else if( (lru_val & 19) == 2 )
    {
        return 2;
    }
    else if( (lru_val & 19) == 18 )
    {
        return 3;
    }
    else if( (lru_val & 37) == 1 )
    {
        return 4;
    }
    else if( (lru_val & 37) == 33 )
    {
        return 5;
    }
    else if( (lru_val & 69) == 5 )
    {
        return 6;
    }
    else if( (lru_val & 69) == 69 )
    {
        return 7;
    }
    else
    {
        return -1;
    }
}

void Cache::extract_address_components(int addr, int *byte_in_line, int *set_address, int *tag)
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
        tags[set_address][line_in_set_index]         = tag;
        cache_status[set_address][line_in_set_index] = 1;
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

        locked_proc_id_mutex = bus->check_ongoing_requests(id, addr, BusRequest::READX);
        bus->readx(id, addr, data);

        line_in_set_index = get_lru_line(set_address);
        cache_status[set_address][line_in_set_index] = 1;
        update_lru(set_address, line_in_set_index);
        tags[set_address][line_in_set_index] = tag;

        bus->release_mutex(id, addr);
        if(locked_proc_id_mutex != -1)
        {
            bus->release_mutex(locked_proc_id_mutex, addr);
        }

        wait();
    }
    else
    {
        log(name(), "write hit on address", addr);
        stats_writehit(id);

        locked_proc_id_mutex = bus->check_ongoing_requests(id, addr, BusRequest::WRITE);
        bus->write(id, addr, data);
        
        cache_status[set_address][line_in_set_index] = 1;
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
    int byte_in_line, set_address, tag;
    extract_address_components(addr, &byte_in_line, &set_address, &tag);

    for(int line_index = 0; line_index < CACHE_NUMBER_OF_LINES_IN_SET; line_index++)
    {
        if(cache_status[set_address][line_index] == 1 && tags[set_address][line_index] == tag)
        {
            cache_status[set_address][line_index] = 0;
            Cache::invalidated_addresses_count++;
        }
    }
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