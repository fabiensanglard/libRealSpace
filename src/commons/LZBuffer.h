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
    uint8_t *DecodeLZWWithByteReaded(const uint8_t *compData, size_t compSize, size_t &uncompSize, size_t &bytesRead);
    uint8_t *DecodeLZWNoStop(const uint8_t* compData, size_t compSize, size_t &uncompSize);
    uint8_t *DecodeLZ77(const uint8_t *compData, size_t compSize, size_t &uncompSize);
    uint8_t *DecodeLZWn_bits(const uint8_t *compData, size_t compSize, size_t &uncompSize, int nbits);
};
