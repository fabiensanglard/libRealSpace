#include "LZBuffer.h"
#include <cstdint>
#include <vector>
#include <cstring>

LZBuffer::LZBuffer() {}

LZBuffer::~LZBuffer() {}

void LZBuffer::Init(uint8_t *data, size_t size) {
    this->data = data;
    this->size = size;
}

uint8_t *LZBuffer::GetData(uint32_t *testsize) { 
    if (!data || size < 4)
        return nullptr;

    uint16_t uncompressedSize = 0;
    size_t inPos = 0;
    if (data[0] == 'L' || data[1] == 'Z') {
        uncompressedSize = data[4] << 16 | data[3] << 8 | data[2];
        size -= 2;
        inPos = 5;
    } else {
        uncompressedSize = data[2] << 16 | data[1] << 8 | data[0];
        inPos = 3;
    }
    *testsize = uncompressedSize;
    uint8_t* output = new uint8_t[uncompressedSize];
    
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

// Helper: read next "width" bits from the stream.
static int getNextCode(const uint8_t* data, size_t compSize, size_t &bitPos, int width) {
    if(bitPos + width > compSize * 8)
        return -1;
    size_t bytePos = bitPos / 8;
    int bitOffset = bitPos % 8;
    
    // Read next 16 bits (at least) to have enough bits.
    uint16_t val = 0;
    if(bytePos < compSize)
        val |= data[bytePos];
    if(bytePos + 1 < compSize)
        val |= data[bytePos+1] << 8;
    
    int code = (val >> bitOffset) & ((1 << width) - 1);
    bitPos += width;
    return code;
}

// UncompressLZWTextbook:
//   compData: pointer to variable-width LZW compressed data buffer
//   compSize: size of compData in bytes.
//   uncompSize: [out] will be set to the uncompressed data size.
// Returns a newly allocated buffer containing the decompressed data.
// The caller must free the returned buffer with delete[].
uint8_t* LZBuffer::UncompressLZWTextbook(const uint8_t* compData, size_t compSize, size_t &uncompSize) {
    // Constants.
    const int MAX_CODE_WIDTH = 12;
    const int CLEAR_CODE = 256;  // Reset dictionary.
    const int STOP_CODE  = 257;  // Terminator.
    const int INITIAL_CODE_WIDTH = 9;
    
    // Initialize dictionary with entries 0 - 255.
    std::vector< std::vector<uint8_t> > dictionary;
    dictionary.reserve(4096);
    for (int i = 0; i < 256; i++) {
        dictionary.push_back(std::vector<uint8_t>(1, static_cast<uint8_t>(i)));
    }
    // Reserve the two special codes.
    dictionary.push_back(std::vector<uint8_t>()); // 256: CLEAR_CODE (dummy).
    dictionary.push_back(std::vector<uint8_t>()); // 257: STOP_CODE (dummy).
    
    int nextCode = 258;  // First new dictionary entry.
    int curCodeWidth = INITIAL_CODE_WIDTH;
    
    size_t bitPos = 0;
    
    std::vector<uint8_t> output;
    std::vector<uint8_t> prevEntry;
    
    while (true) {
        int code = getNextCode(compData, compSize, bitPos, curCodeWidth);
        if (code < 0)
            break;
        
        if (code == CLEAR_CODE) {
            // Reset dictionary.
            dictionary.resize(258);  // Keep initial 258 entries.
            nextCode = 258;
            curCodeWidth = INITIAL_CODE_WIDTH;
            // Read next code after clear.
            code = getNextCode(compData, compSize, bitPos, curCodeWidth);
            if (code < 0)
                break;
            if (code == STOP_CODE)
                break;
            prevEntry = dictionary[code];
            output.insert(output.end(), prevEntry.begin(), prevEntry.end());
            continue;
        }
        
        if (code == STOP_CODE)
            break;
        
        std::vector<uint8_t> currEntry;
        if (code < static_cast<int>(dictionary.size())) {
            currEntry = dictionary[code];
        } else {
            dictionary[code] = std::vector<uint8_t>(1, static_cast<uint8_t>(code));
            currEntry = dictionary[code];
        }
        
        // Output current entry.
        output.insert(output.end(), currEntry.begin(), currEntry.end());
        
        // Add new dictionary entry if possible.
        if (!prevEntry.empty() && nextCode < 4096) {
            std::vector<uint8_t> newEntry = prevEntry;
            newEntry.push_back(currEntry[0]);
            dictionary.push_back(newEntry);
            nextCode++;
            
            // Increase code width if necessary.
            if (nextCode == (1 << curCodeWidth) && curCodeWidth < MAX_CODE_WIDTH)
                curCodeWidth++;
        }
        
        prevEntry = currEntry;
    }
    
    uncompSize = output.size();
    uint8_t* result = new uint8_t[uncompSize];
    memcpy(result, output.data(), uncompSize);
    return result;
}