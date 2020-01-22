#include "memory.h"

int Memory::read(uint32_t addr) {
    log(name(), "read from address", addr);
    wait(100);
    return 0;
}

int Memory::write(uint32_t addr, int data) {
    log(name(), "write to address", addr);
    wait(100);
    return 0;
}