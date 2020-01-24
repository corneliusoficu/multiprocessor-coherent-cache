#ifndef CACHE_H
#define CACHE_H

#include <systemc.h>

#include "psa.h"
#include "cache_if.h"
#include "memory_if.h"
#include "helpers.h"

#define CACHE_LINE_SIZE_BYTES        32
#define CACHE_NUMBER_OF_LINES_IN_SET 8
#define CACHE_NUMBER_OF_SETS         128

class Cache : public cache_if, public sc_module
{

public:
    enum Req
    {
        READ,
        READX,
        WRITE,
        INVALID
    };
    
    sc_port<memory_if> memory;

    Cache(sc_module_name, int);
    ~Cache();

    int cpu_read(uint32_t addr);
    int cpu_write(uint32_t addr, uint32_t data);

private:
    int id;

    int **cache;
    int **tags;
    int **least_recently_updated;

    int bit_mask_byte_in_line = create_mask(0,  4);
    int bit_mask_set_address  = create_mask(5, 11);
    int bit_mask_tag          = create_mask(12, 31);

    void extract_address_components(u_int32_t, int*, int*, int*);
    void initialize_cache_arrays();
    int  create_mask(int, int);
    int  get_index_of_line_in_set(int, int);
    void update_lru(int, int);
    int  get_lru_line(int);
    void handle_cache_read(int, int, int);
    void handle_cache_write(int, int, int, int, int);
    void execute();
};

#endif