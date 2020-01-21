#include <systemc.h>

#include "psa.h"

#define CACHE_LINE_SIZE_BYTES        32
#define CACHE_NUMBER_OF_LINES_IN_SET 8
#define CACHE_NUMBER_OF_SETS         128

SC_MODULE(Cache)
{

public:
    enum Function
    {
        FUNC_READ,
        FUNC_WRITE
    };

    enum RetCode
    {
        RET_READ_DONE,
        RET_WRITE_DONE,
    };

    sc_in<bool>     Port_CLK;
    sc_in<Function> Port_Func;
    sc_in<int>      Port_Addr;
    sc_out<RetCode> Port_Done;
    sc_inout_rv<32> Port_Data;

    SC_CTOR(Cache)
    {
        SC_THREAD(execute);
        sensitive << Port_CLK.pos();
        dont_initialize();
        initialize_cache_arrays();
        cout << bit_mask_byte_in_line << ' ' << bit_mask_set_address << ' ' << bit_mask_tag << '\n';
    }

    ~Cache();

private:
    
    int **cache;
    int **tags;
    int **least_recently_updated;

    int bit_mask_byte_in_line = create_mask(0,  4);
    int bit_mask_set_address  = create_mask(5, 11);
    int bit_mask_tag          = create_mask(12, 31);

    void initialize_cache_arrays();
    int  create_mask(int, int);
    int  get_index_of_line_in_set(int, int);
    void update_lru(int, int);
    int  get_lru_line(int);
    void handle_cache_read(int, int, int);
    void handle_cache_write(int, int, int, int, int);
    void execute();
};