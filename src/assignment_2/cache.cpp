#include "cache.h"

Cache::Cache(sc_module_name name_, int id_) : sc_module(name_), id(id_)
{
    initialize_cache_arrays();
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
    int byte_in_line, set_address, tag;
    extract_address_components(addr, &byte_in_line, &set_address, &tag);

    int line_in_set_index = get_index_of_line_in_set(set_address, tag);
            
    if(line_in_set_index == -1)
    {
        log(name(), "read miss on address", addr);
        stats_readmiss(0);
        wait(100);
        line_in_set_index = get_lru_line(set_address);
        tags[set_address][line_in_set_index] = tag;
        cache[set_address][line_in_set_index * CACHE_LINE_SIZE_BYTES + byte_in_line] = rand() % 1000 + 1;
    }
    else
    {
        log(name(), "read hit on address", addr);
        update_lru(set_address, line_in_set_index);
        stats_readhit(0);
    }
    
    update_lru(set_address, line_in_set_index);
    return 0;
}

int Cache::cpu_write(uint32_t addr, uint32_t data)
{
    int byte_in_line, set_address, tag;
    extract_address_components(addr, &byte_in_line, &set_address, &tag);

    int line_in_set_index = get_index_of_line_in_set(set_address, tag);

    if(line_in_set_index == -1)
    {
        log(name(), "write miss on address", addr);
        stats_writemiss(0);
        line_in_set_index = get_lru_line(set_address);
        tags[set_address][line_in_set_index] = tag;
        //Simulate write in memory 
        log(name(), "read address", addr);
        wait(100);
    }
    else
    {
        log(name(), "write hit on address", addr);
        stats_writehit(0);
        update_lru(set_address, line_in_set_index);
    }

    cache[set_address][line_in_set_index * CACHE_NUMBER_OF_LINES_IN_SET] = data;

    return 0;
}

Cache::~Cache() 
{
    for(int index = 0; index < CACHE_NUMBER_OF_SETS; index++)
    {
        delete[] least_recently_updated[index];
        delete[] cache[index];
        delete[] tags[index];
    }   
}