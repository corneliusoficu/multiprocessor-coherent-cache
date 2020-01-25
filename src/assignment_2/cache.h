#ifndef CACHE_H
#define CACHE_H

#include <systemc.h>

#include "psa.h"
#include "cache_if.h"
#include "bus_if.h"
#include "helpers.h"

#define CACHE_LINE_SIZE_BYTES        32
#define CACHE_NUMBER_OF_LINES_IN_SET 8
#define CACHE_NUMBER_OF_SETS         128

class Cache : public cache_if, public sc_module
{

public:
    sc_in<bool>       port_clk;
    sc_port<Bus_if>   bus;
    sc_in_rv<32>      port_bus_addr;
    sc_in<int>        port_bus_proc;
    sc_in<BusRequest> port_bus_valid;

    bool can_snoop;

    Cache(sc_module_name, int);
    ~Cache();

    SC_HAS_PROCESS(Cache);

    int cpu_read(uint32_t addr);
    int cpu_write(uint32_t addr, uint32_t data);

private:
    int id;

    int     **cache;
    int     **tags;
    int     **cache_status;
    u_int8_t *lru;

    int bit_mask_byte_in_line = create_mask(0,  4);
    int bit_mask_set_address  = create_mask(5, 11);
    int bit_mask_tag          = create_mask(12, 31);

    void extract_address_components(int, int*, int*, int*);
    void initialize_cache_arrays();
    int  create_mask(int, int);
    int  get_index_of_line_in_set(int, int);
    void update_lru(int, int);
    int  get_lru_line(int);
    void handle_cache_read(int, int, int);
    void handle_cache_write(int, int, int, int, int);
    void execute();
    void snoop();
    void handle_snooped_value(int, BusRequest, int);
    void invalidate_cache_copy(int);
};

#endif