#include "LZBuffer.h"
#include <cstdint>
#include <vector>
#include <cstring>
#include <map>
#include <string>
#include "md5.h"

static std::map<std::string, std::pair<uint8_t*, size_t>> lz_buffers;

LZBuffer::LZBuffer() {}

LZBuffer::~LZBuffer() {}

void LZBuffer::init(uint8_t *data, size_t size) {
    this->data = data;
    this->size = size;
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

// Helper: read next "width" bits from the stream.
static int getNextCodeLE(const uint8_t* data, size_t compSize, size_t &bitPos, int width) {
    if(bitPos + width > compSize * 8)
        return -1;
    size_t bytePos = bitPos / 8;
    int bitOffset = bitPos % 8;
    
    // Read next 32 bits (at least) to have enough bits.
    uint32_t val = 0;
    if(bytePos < compSize)
        val |= data[bytePos];
    if(bytePos + 1 < compSize)
        val |= data[bytePos+1] << 8;
    if(bytePos + 2 < compSize)
        val |= data[bytePos+2] << 16;
    if(bytePos + 3 < compSize)
        val |= data[bytePos+3] << 24;
    
    int code = (val >> bitOffset) & ((1 << width) - 1);
    bitPos += width;
    return code;
}

// Décode un buffer compressé en LZW selon les consignes :
//   - Mots de 9 bits au départ, passant à 10, 11 puis 12 bits au fur et à mesure que le dictionnaire grossit.
//   - Le dictionnaire initial comporte 256 entrées (valeurs 8 bits).
//   - Les nouvelles entrées commencent à 258.
//   - Le code 257 indique la fin du flux.
// Décode un buffer compressé en LZW selon les consignes :
//   - Mots de 9 bits au départ, augmentant à 10, 11 puis 12 bits au fur et à mesure que
//     le dictionnaire grossit (jusqu'à 4096 entrées maximum).
//   - Le dictionnaire initial comporte 256 entrées (les valeurs d'un octet).
//   - Les nouvelles entrées commencent à 258.
//   - Le code 256 réinitialise le dictionnaire aux 256 premières entrées.
//   - Le code 257 indique la fin du flux.
uint8_t* LZBuffer::DecodeLZW(const uint8_t* compData, size_t compSize, size_t &uncompSize) {
    std::string hash = ComputeMD5(compData, compSize);

    if (lz_buffers.find(hash) != lz_buffers.end()) {
        uncompSize = lz_buffers[hash].second;
        return lz_buffers[hash].first;
    }
    const int MAX_CODE_WIDTH = 12;
    const int INITIAL_WIDTH = 9;
    const int CLEAR_CODE = 256; // Réinitialise le dictionnaire.
    const int STOP_CODE  = 257; // Fin du flux.

    // Initialiser le dictionnaire avec les 256 valeurs sur 8 bits.
    std::vector< std::vector<uint8_t> > dictionary;
    dictionary.reserve(256+2);
    for (int i = 0; i < 256; i++) {
        dictionary.push_back(std::vector<uint8_t>(1, static_cast<uint8_t>(i)));
    }
    // Réserver deux emplacements (pour conserver la cohérence avec les codes de contrôle).
    dictionary.push_back(std::vector<uint8_t>());
    dictionary.push_back(std::vector<uint8_t>());

    int nextCode = 258;  
    int currentWidth = INITIAL_WIDTH;
    size_t bitPos = 0;
    std::vector<uint8_t> output;
    std::vector<uint8_t> prevEntry;

    // Lire le premier code (il doit être inférieur à 256).
    int code = getNextCodeLE(compData, compSize, bitPos, currentWidth);
    
    if (code == CLEAR_CODE) {
        // Réinitialiser le dictionnaire.
        dictionary.resize(258);
        nextCode = 258;
        currentWidth = INITIAL_WIDTH;
        // Après la réinitialisation, lire le prochain code.
        code = getNextCodeLE(compData, compSize, bitPos, currentWidth);
        if (code < 0 || code == STOP_CODE)
            return nullptr;  // Erreur ou rien à décoder.
    }
    if (code < 0 || code == STOP_CODE)
        return nullptr;  // Erreur ou rien à décoder.
    prevEntry = dictionary[code];
    output.insert(output.end(), prevEntry.begin(), prevEntry.end());
    
    // Lecture des codes suivants et décodage.
    while (true) {
        code = getNextCodeLE(compData, compSize, bitPos, currentWidth);
        if (code == STOP_CODE || code < 0)
            break;
        
        // Vérifier si le code demande une réinitialisation du dictionnaire.
        if (code == CLEAR_CODE) {
            dictionary.resize(258); // Réinitialiser le dictionnaire.
            nextCode = 258;
            currentWidth = INITIAL_WIDTH;
            
            // Lire le prochain code après la réinitialisation.
            code = getNextCodeLE(compData, compSize, bitPos, currentWidth);
            if ( code == STOP_CODE || code < 0)
                break;
            prevEntry = dictionary[code];
            output.insert(output.end(), prevEntry.begin(), prevEntry.end());
            continue;
        }
        
        std::vector<uint8_t> currEntry;
        if (code < static_cast<int>(dictionary.size())) {
            currEntry = dictionary[code];
        } else if (code == nextCode) {
            // Cas particulier : le code n'est pas encore dans le dictionnaire.
            currEntry = prevEntry;
            currEntry.push_back(prevEntry[0]);
        } else {
            continue; // Code non valide.
        }
        
        output.insert(output.end(), currEntry.begin(), currEntry.end());
        
        // Ajouter une nouvelle entrée au dictionnaire, si possible.
        if (!prevEntry.empty() && nextCode < 4096) {
            std::vector<uint8_t> newEntry = prevEntry;
            newEntry.push_back(currEntry[0]);
            dictionary.push_back(newEntry);
            nextCode++;
            
            // Augmenter la largeur de lecture dès que le dictionnaire atteint une taille correspondante.
            if (nextCode == (1 << currentWidth) && currentWidth < MAX_CODE_WIDTH) {
                currentWidth++;
            }
        }
        prevEntry = currEntry;
        if (output.size() == 0x595) {
            printf("LZBuffer::DecodeLZW: output size is 0x594\n");
        }
    }
    size_t bytesRead = bitPos / 8 + (bitPos % 8 != 0 ? 1 : 0);
    if (bytesRead < compSize) {
        printf("LZBuffer::DecodeLZW: bytesRead < compSize\n");
    }
    uncompSize = output.size();
    uint8_t* result = new uint8_t[uncompSize];
    memcpy(result, output.data(), uncompSize);
    std::pair<uint8_t*, size_t> pair = std::make_pair(result, uncompSize);
    lz_buffers[hash] = pair;
    return result;
}

uint8_t* LZBuffer::DecodeLZ77(const uint8_t* compData, size_t compSize, size_t &uncompSize) {
    std::string hash = ComputeMD5(compData, compSize);
    if (lz_buffers.find(hash) != lz_buffers.end()) {
        uncompSize = lz_buffers[hash].second;
        return lz_buffers[hash].first;
    }
    
    // Format compressé LZ77 simple modifié :
    // - Chaque token débute par un flag sur 1 octet.
    // - Si le flag est 0, le token contient directement un octet littéral.
    // - Si le flag est 1, le token de référence contient :
    //       * 4 octets pour la distance (32 bits, little-endian)
    //       * 1 octet pour la longueur (nombre d'octets à recopier).
    // Remarque : la copie peut se chevaucher comme en LZ77 classique.
    std::vector<uint8_t> output;
    size_t pos = 0;
    while (pos < compSize) {
        // Lire le flag
        uint8_t flag = compData[pos++];
        if (flag == 0) {
            // Token littéral : lire le prochain octet et l'ajouter à la sortie.
            if (pos < compSize) {
                output.push_back(compData[pos++]);
            } else {
                break; // données incomplètes
            }
        }
        else if (flag == 1) {
            // Vérifier qu'il reste au moins 5 octets pour lire distance (4 octets) et longueur (1 octet)
            if (pos + 5 > compSize)
                break;
            // Lire 4 octets pour la distance (32 bits little-endian)
            uint32_t distance = *reinterpret_cast<const uint32_t*>(compData + pos);
            pos += 4;
            // Lire 1 octet pour la longueur
            uint8_t length = compData[pos++];
            // Vérifier que la distance est valide (> 0) et que la sortie contient suffisamment de données
            if (distance == 0 || output.size() < distance)
                break;
            size_t start = output.size() - distance;
            // Copier 'length' octets avec chevauchement autorisé
            for (uint8_t i = 0; i < length; i++) {
                output.push_back(output[start + i]);
            }
        }
        else {
            // Flag non reconnu, arrêter le décompressement.
            break;
        }
    }
    
    uncompSize = output.size();
    uint8_t* result = new uint8_t[uncompSize];
    memcpy(result, output.data(), uncompSize);
    lz_buffers[hash] = std::make_pair(result, uncompSize);
    return result;
}

uint8_t* LZBuffer::DecodeLZWn_bits(const uint8_t* compData, size_t compSize, size_t &uncompSize, int nbits) {
    std::string hash = ComputeMD5(compData, compSize);

    if (lz_buffers.find(hash) != lz_buffers.end()) {
        uncompSize = lz_buffers[hash].second;
        return lz_buffers[hash].first;
    }
    const int MAX_CODE_WIDTH = nbits;
    const int INITIAL_WIDTH = nbits;
    const int CLEAR_CODE = 256; // Réinitialise le dictionnaire.
    const int STOP_CODE  = 257; // Fin du flux.

    // Initialiser le dictionnaire avec les 256 valeurs sur 8 bits.
    std::vector< std::vector<uint8_t> > dictionary;
    dictionary.reserve(256+2);
    for (int i = 0; i < 256; i++) {
        dictionary.push_back(std::vector<uint8_t>(1, static_cast<uint8_t>(i)));
    }
    // Réserver deux emplacements (pour conserver la cohérence avec les codes de contrôle).
    dictionary.push_back(std::vector<uint8_t>());
    dictionary.push_back(std::vector<uint8_t>());

    int nextCode = 258;  
    int currentWidth = INITIAL_WIDTH;
    size_t bitPos = 0;
    std::vector<uint8_t> output;
    std::vector<uint8_t> prevEntry;

    // Lire le premier code (il doit être inférieur à 256).
    int code = getNextCodeLE(compData, compSize, bitPos, currentWidth);
    
    if (code == CLEAR_CODE) {
        // Réinitialiser le dictionnaire.
        dictionary.resize(258);
        nextCode = 258;
        currentWidth = INITIAL_WIDTH;
        // Après la réinitialisation, lire le prochain code.
        code = getNextCodeLE(compData, compSize, bitPos, currentWidth);
        if (code < 0 || code == STOP_CODE)
            return nullptr;  // Erreur ou rien à décoder.
    }
    if (code < 0 || code == STOP_CODE)
        return nullptr;  // Erreur ou rien à décoder.
    prevEntry = dictionary[code];
    output.insert(output.end(), prevEntry.begin(), prevEntry.end());
    
    // Lecture des codes suivants et décodage.
    while (true) {
        code = getNextCodeLE(compData, compSize, bitPos, currentWidth);
        if (code == STOP_CODE || code < 0)
            break;
        
        // Vérifier si le code demande une réinitialisation du dictionnaire.
        if (code == CLEAR_CODE) {
            dictionary.resize(258); // Réinitialiser le dictionnaire.
            nextCode = 258;
            currentWidth = INITIAL_WIDTH;
            
            // Lire le prochain code après la réinitialisation.
            code = getNextCodeLE(compData, compSize, bitPos, currentWidth);
            if ( code == STOP_CODE || code < 0)
                break;
            prevEntry = dictionary[code];
            output.insert(output.end(), prevEntry.begin(), prevEntry.end());
            continue;
        }
        
        std::vector<uint8_t> currEntry;
        if (code < static_cast<int>(dictionary.size())) {
            currEntry = dictionary[code];
        } else if (code == nextCode) {
            // Cas particulier : le code n'est pas encore dans le dictionnaire.
            currEntry = prevEntry;
            currEntry.push_back(prevEntry[0]);
        } else {
            continue; // Code non valide.
        }
        
        output.insert(output.end(), currEntry.begin(), currEntry.end());
        
        // Ajouter une nouvelle entrée au dictionnaire, si possible.
        if (!prevEntry.empty() && nextCode < 4096) {
            std::vector<uint8_t> newEntry = prevEntry;
            newEntry.push_back(currEntry[0]);
            dictionary.push_back(newEntry);
            nextCode++;
            
            // Augmenter la largeur de lecture dès que le dictionnaire atteint une taille correspondante.
            if (nextCode == (1 << currentWidth) && currentWidth < MAX_CODE_WIDTH) {
                currentWidth++;
            }
        }
        prevEntry = currEntry;
        if (output.size() == 0x595) {
            printf("LZBuffer::DecodeLZW: output size is 0x594\n");
        }
    }
    
    uncompSize = output.size();
    uint8_t* result = new uint8_t[uncompSize];
    memcpy(result, output.data(), uncompSize);
    std::pair<uint8_t*, size_t> pair = std::make_pair(result, uncompSize);
    lz_buffers[hash] = pair;
    return result;
}