#pragma once
#include <cstddef>
#include <cstdint>

class LZBuffer {
private:
    uint8_t* data;
    size_t size;
public:
    LZBuffer();
    ~LZBuffer();

    void init(uint8_t* data, size_t size);
    uint8_t *DecodeLZW(const uint8_t *compData, size_t compSize, size_t &uncompSize);
};
