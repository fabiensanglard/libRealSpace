#include "RLEBuffer.h"

/**/
uint8_t * RLEBuffer::Decompress(uint8_t * compressedBuffer, size_t size, size_t &decompressed_size) {
    std::vector<std::pair<uint8_t, uint8_t>> decompressed;
    size_t i = 0;
    decompressed_size = 0;
    while (i + 1 < size) { // On s'assure de lire une paire complète [count, value]
        uint8_t count = compressedBuffer[i++];
        uint8_t value = compressedBuffer[i++];
        decompressed.push_back({count, value});
        decompressed_size += count;
    }
    uint8_t * result = new uint8_t[decompressed_size]; // On alloue un tableau de taille exacte
    size_t pos = 0;
    for (auto d: decompressed) {
        for (int j = 0; j < d.first; j++) {
            result[pos++] = d.second;
        }
    }
    return result;
}
/**/

/*
uint8_t * RLEBuffer::Decompress(uint8_t * compressedBuffer, size_t size, size_t &decompressed_size) {
    std::vector<uint8_t> decompressed;
    size_t i = 0;
    decompressed_size = 0;
    while (i + 1 < size) { // On s'assure de lire une paire complète [count, value]
        uint8_t count = compressedBuffer[i++];
        if (count == 128) {
            // nop do nothing
        } else if (count > 128) {
            // This is a repeated byte
            count = 256 - count;
            for (int j = 0; j <= count; ++j) {
                decompressed.push_back(compressedBuffer[i++]);
                decompressed_size += 1;
            }
        } else {
            // These are literal bytes
            for (int j = 0; j <= count; ++j) {
                decompressed.push_back(compressedBuffer[i+j+1]);
                decompressed_size += 1;
            }
            i += count;
        }
    }
    if (decompressed_size != decompressed.size()) {
        printf("Error decompressing RLE buffer\n");
    }
    uint8_t * result = new uint8_t[decompressed_size]; // On alloue un tableau de taille exacte
    size_t pos = 0;
    for (auto d: decompressed) {
        result[pos++] = d;
    }
    return result;
}
/**/