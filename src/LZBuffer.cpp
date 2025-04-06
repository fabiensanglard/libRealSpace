#include "LZBuffer.h"

LZBuffer::LZBuffer() {}

LZBuffer::~LZBuffer() {}

void LZBuffer::Init(uint8_t *data, size_t size) {
    this->data = data;
    this->size = size;
}

uint8_t *LZBuffer::GetData(uint32_t uncompressedSize) { 
    if (!data || size < 4)
        return nullptr;

    uint8_t* output = new uint8_t[uncompressedSize];
    
    size_t inPos = 6;
    size_t outPos = 0;
    
    while (inPos < size && outPos < uncompressedSize) {
        // Read flag byte: each bit indicates literal (1) or back-reference (0)
        uint8_t flag = data[inPos++];
        for (int bit = 0; bit < 8 && outPos < uncompressedSize; bit++) {
            if (flag & (1 << bit)) {
                // Literal byte
                if (inPos >= size)
                    break;
                output[outPos++] = data[inPos++];
            } else {
                // Back-reference token: 2 bytes containing offset and length info.
                if (inPos + 1 >= size)
                    break;
                uint8_t b1 = data[inPos++];
                uint8_t b2 = data[inPos++];
                uint16_t token = (b1 << 8) | b2;
                // Upper 4 bits: length (stored as value-3), lower 12 bits: offset.
                uint16_t length = (token >> 12) + 3;
                uint16_t offset = token & 0x0FFF;
                for (uint16_t i = 0; i < length && outPos < uncompressedSize; i++) {
                    // Make sure we don't read before beginning of output.
                    if (outPos < offset + 1)
                        break;
                    output[outPos] = output[outPos - offset - 1];
                    outPos++;
                }
            }
        }
    }
    
    return output;
}
