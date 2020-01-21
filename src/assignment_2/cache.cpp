#include "cache.h"

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

void Cache::handle_cache_read(int set_address, int tag, int byte_in_line)
{
    int line_in_set_index = get_index_of_line_in_set(set_address, tag);
            
    if(line_in_set_index == -1)
    {
        stats_readmiss(0);
        //simulate read from memory and store in cache
        wait(99);
        line_in_set_index = get_lru_line(set_address);
        tags[set_address][line_in_set_index] = tag;
        cache[set_address][line_in_set_index * CACHE_LINE_SIZE_BYTES + byte_in_line] = rand() % 1000 + 1;
    }
    else
    {
        update_lru(set_address, line_in_set_index);
        stats_readhit(0);
    }
    
    update_lru(set_address, line_in_set_index);
    
    Port_Data.write(cache[set_address][line_in_set_index * CACHE_LINE_SIZE_BYTES + byte_in_line]);
    Port_Done.write(RET_READ_DONE);
    
    wait();
    Port_Data.write("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ");
}

void Cache::handle_cache_write(int addr, int set_address, int tag, int byte_in_line, int data)
{
    int line_in_set_index = get_index_of_line_in_set(set_address, tag);

    if(line_in_set_index == -1)
    {
        stats_writemiss(0);
        line_in_set_index = get_lru_line(set_address);
        tags[set_address][line_in_set_index] = tag;
        //Simulate write in memory 
        wait(99);
    }
    else
    {
        stats_writehit(0);
        update_lru(set_address, line_in_set_index);
        wait();
    }

    cache[set_address][line_in_set_index * CACHE_NUMBER_OF_LINES_IN_SET] = data;
    Port_Done.write(RET_WRITE_DONE);
}

void Cache::execute()
{
    while (true)
    {
        wait(Port_Func.value_changed_event());

        Function f = Port_Func.read();
        int addr   = Port_Addr.read();

        int byte_in_line = (addr & bit_mask_byte_in_line);       // Obtaining value for bits 0 - 4, no shifting required
        int set_address  = (addr & bit_mask_set_address) >> 5; // Shifting to right to obtain value for bits 5  - 11
        int tag          = (addr & bit_mask_tag) >> 12;        // Shifting to right to obtain value for bits 12 - 31

        if (f == FUNC_WRITE)
        {
            cout << sc_time_stamp() << ": Cache received write" << endl;
            int data = Port_Data.read().to_int();
            handle_cache_write(addr, set_address, tag, byte_in_line, data);
        }
        else
        {
            cout << sc_time_stamp() << ": Cache received read" << endl;
            handle_cache_read(set_address, tag, byte_in_line);
        }
    }   
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