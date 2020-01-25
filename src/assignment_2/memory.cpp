#include "memory.h"

int Memory::read(uint32_t addr) {
    wait(100);
    log(name(), "read from address", addr);
    return 0;
}

int Memory::write(uint32_t addr, int data) {
    wait(100);
    log(name(), "written to address", addr);
    return 0;
}