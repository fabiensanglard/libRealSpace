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

uint8_t * RLEBuffer::Decompress6Bit(uint8_t * compressedBuffer, size_t size, size_t &decompressed_size) {
    std::vector<std::pair<uint8_t, uint8_t>> runs;
    decompressed_size = 0;
    size_t totalBits = size * 8;
    size_t bitPos = 0;

    auto extract6 = [compressedBuffer, size](size_t pos) -> uint8_t {
        size_t byteIndex = pos / 8;
        int bitOffset = pos % 8;
        uint16_t twoBytes = compressedBuffer[byteIndex] << 8;
        if (byteIndex + 1 < size) {
            twoBytes |= compressedBuffer[byteIndex + 1];
        }
        int shift = 16 - bitOffset - 6;
        return (twoBytes >> shift) & 0x3F;
    };

    while (bitPos + 12 <= totalBits) {
        uint8_t count = extract6(bitPos);
        bitPos += 6;
        uint8_t value = extract6(bitPos);
        bitPos += 6;
        runs.push_back({count, value});
        decompressed_size += count;
    }

    uint8_t * result = new uint8_t[decompressed_size];
    size_t pos = 0;
    for (const auto &run : runs) {
        for (int i = 0; i < run.first; i++) {
            result[pos++] = run.second;
        }
    }
    return result;
}

uint8_t * RLEBuffer::DecompressPacketBit6(uint8_t * compressedBuffer, size_t size, size_t &decompressed_size) {
    std::vector<uint8_t> output;
    size_t totalBits = size * 8;
    size_t bitPos = 0;

    auto extract6 = [compressedBuffer, size](size_t pos) -> uint8_t {
        size_t byteIndex = pos / 8;
        int bitOffset = pos % 8;
        uint16_t twoBytes = compressedBuffer[byteIndex] << 8;
        if (byteIndex + 1 < size) {
            twoBytes |= compressedBuffer[byteIndex + 1];
        }
        int shift = 16 - bitOffset - 6;
        return (twoBytes >> shift) & 0x3F;
    };

    while (bitPos + 6 <= totalBits) {
        // Lire le mot d'en-tête en 6 bits et l'interpréter en entier signé (-32..31).
        uint8_t rawHeader = extract6(bitPos);
        bitPos += 6;
        int8_t header = (rawHeader & 0x20) ? (rawHeader | 0xC0) : rawHeader; // Extension du signe sur 6 bits

        if (header >= 0) {
            // Séquence littérale : header + 1 valeurs littérales suivent.
            size_t literalCount = header + 1;
            if (bitPos + literalCount * 6 > totalBits)
                break; // Erreur de format ou données incomplètes
            for (size_t i = 0; i < literalCount; i++) {
                uint8_t literal = extract6(bitPos);
                bitPos += 6;
                output.push_back(literal);
            }
        }
        else {
            // Séquence de répétition : la prochaine valeur doit être répétée (-header + 1) fois.
            size_t runCount = -header + 1;
            if (bitPos + 6 > totalBits)
                break; // Erreur de format
            uint8_t value = extract6(bitPos);
            bitPos += 6;
            output.insert(output.end(), runCount, value);
        }
    }

    decompressed_size = output.size();
    uint8_t * result = new uint8_t[decompressed_size];
    std::copy(output.begin(), output.end(), result);
    return result;
}

uint8_t * RLEBuffer::DecompressPacketBit(uint8_t * compressedBuffer, size_t size, size_t &decompressed_size)
{
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
    uint8_t * result = new uint8_t[decompressed_size]; // On alloue un tableau de taille exacte
    size_t pos = 0;
    for (auto d: decompressed) {
        result[pos++] = d;
    }
    return result;
}