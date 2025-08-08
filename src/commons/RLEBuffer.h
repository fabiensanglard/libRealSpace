#ifndef RLE_BUFFER_H
#define RLE_BUFFER_H

#include <vector>
#include <cstddef>
#include <cstdint>

class RLEBuffer {
public:
    // Décompresse un buffer compressé avec l'algorithme RLE.
    // Le format attendu est une séquence de paires [count, value] :
    // Par exemple, { 3, 0xAA } produira { 0xAA, 0xAA, 0xAA }.
    // Si le buffer a un nombre impair d'octets, la dernière paire incomplète est ignorée.
    uint8_t *Decompress(uint8_t * compressedBuffer, size_t size, size_t &uncompressedSize);
    uint8_t *Decompress6Bit(uint8_t *compressedBuffer, size_t size, size_t &decompressed_size);
    uint8_t *DecompressPacketBit6(uint8_t *compressedBuffer, size_t size, size_t &decompressed_size);
    uint8_t *DecompressPacketBit(uint8_t *compressedBuffer, size_t size, size_t &decompressed_size);
};

#endif // RLE_BUFFER_H