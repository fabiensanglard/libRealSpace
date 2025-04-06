#pragma once
#include <cstdint>

class LZBuffer {
private:
    uint8_t* data;
    size_t size;
public:
    LZBuffer();
    ~LZBuffer();

    void Init(uint8_t* data, size_t size);
    uint8_t* GetData(uint32_t uncompressedSize);
};
