#ifndef DATATYPES_H
#define DATATYPES_H

enum BusRequest
{
    INVALID,
    READ,
    WRITE,
    READX
};

typedef struct
{
    sc_mutex   access_mutex;
    int        address;
    BusRequest request_type;
} RequestContent;

#endif

