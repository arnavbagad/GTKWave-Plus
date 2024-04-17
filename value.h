#pragma once
 

#include <string>
#include <stdint.h>

class Value {
public:
    uint64_t val;
    uint64_t ndef;

    // Constructor
    Value(uint64_t value, uint64_t undefined) : val(value), ndef(undefined) {}

    std::string toString();
};
