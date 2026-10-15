//
//  RSMap.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RSArea.h"
#include "../engine/Config.hpp"
#include "../engine/SCRenderer.h"
#include <cmath>
#include <random>
std::mt19937 rng(42);

int randInt (int max) {
    return std::uniform_int_distribution<int>(0, max - 1)(rng);
}
float randFloat(float min, float max) {
    return std::uniform_real_distribution<float>(min, max)(rng);
}

RSArea::RSArea() {

}
RSArea::~RSArea() {
    //delete archive;
    for (auto overlay : objectOverlay) {
        delete[] overlay.vertices;
        overlay.vertices = nullptr;
        overlay.nbTriangles = 0;
    }
    while (!textures.empty()) {
        RSMapTextureSet *set = textures.back();
        textures.pop_back();
        delete set;
    }
}

#define OBJ_ENTRY_SIZE 0x46
#define OBJ_ENTRY_NUM_OBJECTS_FIELD 0x2
void RSArea::ParseObjects() {
    /*
        The OBJ file seems to have a pattern:
            It is a PAK archive
            For each entry in the PAK
            short: num records
                X records of length 0x46
            Record format :
                7 bytes for name
                1 bytes unknown (sometimes 0x00 sometimes 0xC3
                4 bytes unknown
                12 bytes for coordinates ?
    */
    PakEntry *objectsFilesLocation = archive->GetEntry(5);
    objectsFilesLocation = archive->GetEntry(5);

    PakArchive objectFiles;
    objectFiles.InitFromRAM("PAK Objects from RAM", objectsFilesLocation->data, objectsFilesLocation->size);

    for (size_t i = 0; i < objectFiles.GetNumEntries(); i++) {
        PakEntry *entry = objectFiles.GetEntry(i);
        if (entry->size == 0)
            continue;
        entry = objectFiles.GetEntry(i);
        ByteStream sizeGetter(entry->data, entry->size);
        uint16_t numObjs = sizeGetter.ReadUShort();

        for (int j = 0; j < numObjs; j++) {

            ByteStream reader(entry->data + OBJ_ENTRY_NUM_OBJECTS_FIELD + OBJ_ENTRY_SIZE * j, entry->size-OBJ_ENTRY_NUM_OBJECTS_FIELD - OBJ_ENTRY_SIZE * j);
            MapObject mapObject;

            for (int k = 0; k < 8; k++)
                mapObject.name[k] = reader.ReadByte();
            mapObject.name[8] = 0;

            mapObject.unknowns.push_back(reader.ReadByte());
            mapObject.unknowns.push_back(reader.ReadByte());
            mapObject.unknowns.push_back(reader.ReadByte());
            mapObject.unknowns.push_back(reader.ReadByte());
            mapObject.unknowns.push_back(reader.ReadByte());

            for (int k = 0; k < 8; k++)
                mapObject.destroyedName[k] = reader.ReadByte();
            mapObject.destroyedName[8] = 0;

            //mapObject.unknowns.push_back(reader.ReadByte());
            mapObject.unknowns.push_back(reader.ReadByte());
            mapObject.position.x = reader.ReadFixedFloatLE() * BLOCK_COORD_SCALE;
            mapObject.position.z = -reader.ReadFixedFloatLE() * BLOCK_COORD_SCALE;
            mapObject.position.y = reader.ReadFixedFloatLE() * HEIGH_MAP_SCALE;
            for (int row = 0; row < 3; row++) {
                for (int column = 0; column < 3; column++) {
                    mapObject.orientation[row][column] = reader.ReadFixedFloatLE();
                }
            }
            mapObject.entity = nullptr;
            objects.push_back(mapObject);
        }
    }
}

void RSArea::ParseTriFile(PakEntry *entry) {
    if (entry->size <= 0)
        return;

    AreaOverlay overTheMapIsTheRunway;
    size_t      read = 0;
    ByteStream  stream(entry->data, entry->size);

    // --- en-tête : 4 × u16 LE ---
    int numvertice = stream.ReadUShort(); read += 2;
    int nbpoly     = stream.ReadUShort(); read += 2;
    int param2     = stream.ReadUShort(); read += 2;
    int param3     = stream.ReadUShort(); read += 2;
    overTheMapIsTheRunway.param2 = (uint16_t)param2;
    overTheMapIsTheRunway.param3 = (uint16_t)param3;

    // --- 1. sommets : numvertice × 13 octets (i32 X, i32 Z, i32 Y, u8 pad) ---
    AoVPoints *vertices = new AoVPoints[numvertice];
    overTheMapIsTheRunway.lx = overTheMapIsTheRunway.ly = 0;
    overTheMapIsTheRunway.hx = overTheMapIsTheRunway.hy = 0;

    for (int i = 0; i < numvertice; i++) {
        AoVPoints *v = &vertices[i];
        v->x   = stream.ReadFixedFloatLE() * BLOCK_COORD_SCALE; read += 4;
        v->z   = -stream.ReadFixedFloatLE() * BLOCK_COORD_SCALE; read += 4;
        v->y   = stream.ReadFixedFloatLE() * HEIGH_MAP_SCALE;   read += 4;
        v->pad = stream.ReadByte();                             read += 1;

        if (i == 0) {
            overTheMapIsTheRunway.lx = overTheMapIsTheRunway.hx = v->x;
            overTheMapIsTheRunway.ly = overTheMapIsTheRunway.hy = v->z;
        }
        overTheMapIsTheRunway.lx = v->x < overTheMapIsTheRunway.lx ? v->x : overTheMapIsTheRunway.lx;
        overTheMapIsTheRunway.ly = v->z < overTheMapIsTheRunway.ly ? v->z : overTheMapIsTheRunway.ly;
        overTheMapIsTheRunway.hx = v->x > overTheMapIsTheRunway.hx ? v->x : overTheMapIsTheRunway.hx;
        overTheMapIsTheRunway.hy = v->z > overTheMapIsTheRunway.hy ? v->z : overTheMapIsTheRunway.hy;
        overTheMapIsTheRunway.verticesVec.push_back(*v);
    }
    overTheMapIsTheRunway.vertices = vertices;

    // --- 2. polygones : nbpoly × 16 octets ---
    overTheMapIsTheRunway.nbTriangles = 0;
    for (int i = 0; i < nbpoly; i++) {
        AreaOverlayTriangles aot;
        aot.flag0          = stream.ReadByte();   read += 1;
        aot.verticesIdx[0] = stream.ReadUShort(); read += 2;
        aot.verticesIdx[1] = stream.ReadUShort(); read += 2;
        aot.verticesIdx[2] = stream.ReadUShort(); read += 2;
        aot.type           = stream.ReadByte();   read += 1;
        aot.color          = stream.ReadUShort(); read += 2;
        for (int k = 0; k < 3; k++) {
            aot.uv[k].u = stream.ReadByte();
            aot.uv[k].v = stream.ReadByte();
        }
        read += 6;

        for (int k = 0; k < 3; k++) {
            if (aot.verticesIdx[k] >= numvertice) {
                printf("Warning: TRI vertex index out of bounds: %u >= %d\n",
                       aot.verticesIdx[k], numvertice);
                aot.verticesIdx[k] = 0;
            }
        }
        overTheMapIsTheRunway.trianles[overTheMapIsTheRunway.nbTriangles++] = aot;
    }

    // --- 3. table param3 : param3 × u16 (runs [count, polyIdx...], 0xFFFF = liste vide) ---
    overTheMapIsTheRunway.param3Table.reserve(param3);
    for (int i = 0; i < param3; i++) {
        overTheMapIsTheRunway.param3Table.push_back(stream.ReadUShort());
        read += 2;
    }

    // --- 4. grille : param2*param2 × u16 (chaque case = index dans param3Table) ---
    int ncell = param2 * param2;
    overTheMapIsTheRunway.grid.reserve(ncell);
    for (int i = 0; i < ncell; i++) {
        overTheMapIsTheRunway.grid.push_back(stream.ReadUShort());
        read += 2;
    }

    // --- drawList : union ordonnée des polygones référencés ; repli sur 0..nbpoly-1 ---
    std::vector<char> seen(nbpoly, 0);
    for (int c = 0; c < ncell; c++) {
        uint16_t g = overTheMapIsTheRunway.grid[c];
        if (g >= overTheMapIsTheRunway.param3Table.size())          continue;
        if (overTheMapIsTheRunway.param3Table[g] == 0xFFFF)         continue;   // case vide
        uint16_t count = overTheMapIsTheRunway.param3Table[g];
        for (uint16_t k = 0; k < count && (g + 1 + k) < overTheMapIsTheRunway.param3Table.size(); k++) {
            uint16_t idx = overTheMapIsTheRunway.param3Table[g + 1 + k];
            if (idx < nbpoly && !seen[idx]) {
                seen[idx] = 1;
                overTheMapIsTheRunway.drawList.push_back(idx);
            }
        }
    }
    if (overTheMapIsTheRunway.drawList.empty())
        for (int i = 0; i < nbpoly; i++)
            overTheMapIsTheRunway.drawList.push_back((uint16_t)i);

    if (read != entry->size)
        printf("Warning: TRI file read mismatch: %zu != %zu\n", read, entry->size);

    objectOverlay.push_back(overTheMapIsTheRunway);
}

void RSArea::ParseTrigo() {

    PakEntry *entry;
    entry = archive->GetEntry(4);

    PakArchive triFiles;
    triFiles.InitFromRAM(".TRI", entry->data, entry->size);

    for (size_t i = 0; i < triFiles.GetNumEntries(); i++) {

        PakEntry *entry = triFiles.GetEntry(i);
        ParseTriFile(entry);
    }
}

#define LAND_TYPE_SEA 0
#define LAND_TYPE_DESERT 1
#define LAND_TYPE_GROUND 2
#define LAND_TYPE_SAVANNAH 3
#define LAND_TYPE_TAIGA 4
#define LAND_TYPE_TUNDRA 5
#define LAND_TYPE_SNOW 6

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)                                                                                             \
    (byte & 0x80 ? 1 : 0), (byte & 0x40 ? 1 : 0), (byte & 0x20 ? 1 : 0), (byte & 0x10 ? 1 : 0), (byte & 0x08 ? 1 : 0), \
        (byte & 0x04 ? 1 : 0), (byte & 0x02 ? 1 : 0), (byte & 0x01 ? 1 : 0)

// A lod features
// A block features either 25, 100 or 400 vertex
void RSArea::ParseBlocks(size_t lod, PakEntry *entry, size_t blockDim) {

    PakArchive blocksPAL;
    blocksPAL.InitFromRAM("BLOCKS", entry->data, entry->size);
    for (size_t i = 0; i < blocksPAL.GetNumEntries(); i++) { // Iterate over the BLOCKS_PER_MAP block entries.

        // SRC Asset Block
        PakEntry *blockEntry = blocksPAL.GetEntry(i);

        // DST custom block
        AreaBlock *block = &blocks[lod][i];

        block->sideSize = static_cast<int32_t>(blockDim);

        ByteStream vertStream(blockEntry->data, blockEntry->size);
        for (size_t vertexID = 0; vertexID < blockDim * blockDim; vertexID++) {

            MapVertex *vertex = &block->vertice[vertexID];

            int16_t height;
            height = vertStream.ReadShort();
            
            vertex->flag = vertStream.ReadByte();
            vertex->type = vertStream.ReadByte();

            vertex->upperImageID = vertStream.ReadByte();
            vertex->lowerImageID = vertStream.ReadByte();
            vertex->height = height;
            Renderer.terrainColor(vertex->type, vertex->flag, vertex->color);

            vertex->v.y = height * HEIGH_MAP_SCALE;

            vertex->v.x = i % BLOCK_PER_MAP_SIDE * BLOCK_WIDTH + (vertexID % blockDim) / (float)(blockDim)*BLOCK_WIDTH;
            vertex->v.z = i / BLOCK_PER_MAP_SIDE * BLOCK_WIDTH + (vertexID / blockDim) / (float)(blockDim)*BLOCK_WIDTH;
            vertex->v.x += -BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;
            vertex->v.z += -BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;

        }
    }
}

void RSArea::ParseElevations(void) {

    PakEntry *entry;

    entry = archive->GetEntry(6);

    ByteStream stream(entry->data, entry->size);

    for (size_t i = 0; i < BLOCKS_PER_MAP; i++) {
        elevation[i] = stream.ReadUShort();
    }
}

void RSArea::ParseHeightMap(void) {

    PakEntry *entry;

    entry = archive->GetEntry(1);
    PakArchive fullPak;
    fullPak.InitFromRAM("FULLSIZE", entry->data, entry->size);
    ParseBlocks(BLOCK_LOD_MAX, entry, 20);

    entry = archive->GetEntry(2);
    PakArchive medPak;
    medPak.InitFromRAM("MED SIZE", entry->data, entry->size);
    ParseBlocks(BLOCK_LOD_MED, entry, 10);

    entry = archive->GetEntry(3);
    PakArchive smallPak;
    smallPak.InitFromRAM("SMALSIZE", entry->data, entry->size);
    ParseBlocks(BLOCK_LOD_MIN, entry, 5);

    BuildSkirts();
    generateClouds(randInt(500), 15000, 15.0f);
    
}

RSImage *RSArea::GetImageByID(size_t ID) {
    if (ID >= textures[0]->GetNumImages())
        return textures[1]->GetImageById(ID-textures[0]->GetNumImages());
    return textures[0]->GetImageById(ID); 
}

void RSArea::InitFromPAKFileName(const char *pakFilename) {

    strcpy(name, pakFilename);

    // Check the PAK has 5 entries
    FileData *fileData = assetsManager.GetFileData(pakFilename);
    if (fileData == nullptr) {
        printf("Error: Cannot find area file %s\n", pakFilename);
        return;
    }
    this->InitFromRam(pakFilename, fileData->data, fileData->size);
}
void RSArea::InitFromZipFileName(std::string zipFilename) {
    TreEntry *treEntry = assetsManager.GetEntryByName(zipFilename);
    PKWareDecompressor decompressor;
    if (treEntry == nullptr) {
        printf("Error: Cannot find area file %s in TRE\n", zipFilename.c_str());
        return;
    }
    size_t uncompSize = 0;
    uint8_t *data = decompressor.DecompressPKWare(treEntry->data, treEntry->size, uncompSize);
    this->InitFromRam(zipFilename.c_str(), data, uncompSize);
}
void RSArea::InitFromRam(const char *pakFilename, uint8_t *data, size_t size) {
    this->archive = new PakArchive();
    this->archive->InitFromRAM(pakFilename, data, size);
    this->objects.clear();
    this->objects.shrink_to_fit();

    this->objectOverlay.clear();
    this->textures.clear();
    // Check that we have 6 entries.
    if (archive->GetNumEntries() != 7) {
        printf("***Error: An area PAK file should have 7 files:\n");
        printf("        - IFF file.\n");
        printf("        - 1 file containing records of size 2400.\n");
        printf("        - 1 file containing records of size 600 (mipmaps of the 2400 records ?)\n");
        printf("        - 1  file containing records of size 150 (mipmaps of the 600  records ?)\n");
        printf("        - 1 file containing the map 3D data (MAURITAN.TRI).\n");
        printf("        - 1 file containing the objects locations on the map (MAURITAN.OBJ).\n"); // COMPLETELY REVERSE
                                                                                                  // ENGINEERED !!!!
        printf("        - 1 file containing MAURITAN.AVG (I assume Average of something ?)\n");
        return;
    }

    // Load the textures from the PAKs (TXMPACK.PAK and ACCPACK.PAK) within TEXTURES.TRE.
    /*
            Note: This is the bruteforce approach and not very good:
                I feel like the right way would be to be able to parse the AREA info
                (the first IFF entry which seems to contain textures references.)
                And load only those from the TXM PACK. This is probably how they did
                it on a machine with only 4MB of RAM.

    */

    // Find the texture PAKS.
    TreEntry *treEntry = NULL;
    RSMapTextureSet *set;

    treEntry = assetsManager.GetEntryByName(assetsManager.txm_filename);
    PakArchive txmPakArchive;
    txmPakArchive.InitFromRAM("TXMPACK", treEntry->data, treEntry->size);
    set = new RSMapTextureSet();
    set->InitFromPAK(&txmPakArchive);
    textures.push_back(set);

    // ACCPACK.PAK seems to contain runway textures
    treEntry = assetsManager.GetEntryByName(assetsManager.acc_filename);
    PakArchive accPakArchive;
    accPakArchive.InitFromRAM("ACCPACK.PAK", treEntry->data, treEntry->size);
    set = new RSMapTextureSet();
    set->InitFromPAK(&accPakArchive);
    textures.push_back(set);

    // Parse the meta datas.
    ParseElevations();
    //ParseMetadata();
    PakEntry *entry = archive->GetEntry(0);

    parseTERA(entry->data, entry->size);

    ParseObjects();
    ParseTrigo();

    ParseHeightMap();
}
float RSArea::getGroundLevel(int BLOC, float x, float y) {
    if (BLOC < 0 || BLOC >= BLOCKS_PER_MAP)
        return 0;

    const AreaBlock& block = blocks[0][BLOC];
    const int gridSize = block.sideSize; // 20 pour LOD max
    
    // Calculer les limites du bloc
    int blocIndexX = BLOC % BLOCK_PER_MAP_SIDE;
    int blocIndexY = BLOC / BLOCK_PER_MAP_SIDE;
    float blocMinX = blocIndexX * BLOCK_WIDTH - BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;
    float blocMinZ = blocIndexY * BLOCK_WIDTH - BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;
    
    // Trouver le vertex le plus proche directement en comparant les coordonnées réelles
    float bestDistance = (std::numeric_limits<float>::max)();
    int bestIndex = 0;
    
    for (int i = 0; i < gridSize * gridSize; i++) {
        const MapVertex& v = block.vertice[i];
        float dx = x - v.v.x;
        float dz = y - v.v.z;
        float distanceSquared = dx * dx + dz * dz;
        
        if (distanceSquared < bestDistance) {
            bestDistance = distanceSquared;
            bestIndex = i;
        }
    }
    
    // Récupérer les indices de grille du vertex le plus proche
    int i0 = bestIndex % gridSize;
    int j0 = bestIndex / gridSize;
    
    // Si on est sur un bord, utiliser directement la hauteur du vertex
    if (i0 >= gridSize - 1 || j0 >= gridSize - 1) {
        return block.vertice[bestIndex].v.y + 5.0f;
    }
    
    int i1 = i0 + 1;
    int j1 = j0 + 1;
    
    // Récupérer les 4 vertices du quad pour interpolation
    const MapVertex* v00 = &block.vertice[j0 * gridSize + i0];
    const MapVertex* v10 = &block.vertice[j0 * gridSize + i1];
    const MapVertex* v01 = &block.vertice[j1 * gridSize + i0];
    const MapVertex* v11 = &block.vertice[j1 * gridSize + i1];
    
    // Calculer les facteurs d'interpolation en utilisant les coordonnées réelles des vertices
    float cellWidth = v10->v.x - v00->v.x;
    float cellDepth = v01->v.z - v00->v.z;
    
    float fx = 0.0f, fz = 0.0f;
    if (cellWidth > 0.0001f) {
        fx = (x - v00->v.x) / cellWidth;
    }
    if (cellDepth > 0.0001f) {
        fz = (y - v00->v.z) / cellDepth;
    }
    
    // Clamper les facteurs entre 0 et 1
    fx = (std::max)(0.0f, (std::min)(1.0f, fx));
    fz = (std::max)(0.0f, (std::min)(1.0f, fz));
    
    // Interpolation bilinéaire
    float h0 = v00->v.y * (1.0f - fx) + v10->v.y * fx;
    float h1 = v01->v.y * (1.0f - fx) + v11->v.y * fx;
    float height = h0 * (1.0f - fz) + h1 * fz;
    
    return height + 5.0f;
}
/*float RSArea::getGroundLevel(int BLOC, float x, float y) {
    if (BLOC < 0 || BLOC >= BLOCKS_PER_MAP)
        return 0;

    bool newcode = true;
    if (newcode) {
        // x et y correspondent respectivement à x et z des vertices
        float bestDistance = (std::numeric_limits<float>::max)();
        float resultY = 0;
        int cellule_id = 0;
        for (int i = 0; i < 400; i++) {
            const auto &vertex = blocks[0][BLOC].vertice[i].v;
            float dx = x - vertex.x;
            float dz = y - vertex.z;
            float distanceSquared = dx * dx + dz * dz;

            if (distanceSquared < bestDistance) {
                bestDistance = distanceSquared;
                resultY = vertex.y;
                cellule_id = i;
            }
        }
        return resultY+5;
    }
    

    if (BLOC < 0 || BLOC >= BLOCKS_PER_MAP)
        return 0;

    int verticeIndex = 0;
    int centerX = 0;
    int centerY = 0;
    int vX = static_cast<int>((x) + centerX);
    int vY = static_cast<int>((y) + centerY);

    vX = vX / 1000;
    vY = vY / 1000;

    vX = vX * 1000;
    vY = vY * 1000;

    for (int i = 0; i < 400; i++) {
        if ((blocks[0][BLOC].vertice[i].v.x >= vX) && (blocks[0][BLOC].vertice[i].v.z >= vY)) {
            return (blocks[0][BLOC].vertice[i].v.y + 5);
        }
    }
    return (blocks[0][BLOC].vertice[0].v.y);

    
}*/

float RSArea::getY(float x, float z) {

    int centerX = BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;
    int centerY = BLOCK_WIDTH * BLOCK_PER_MAP_SIDE_DIV_2;
    int blocX = (int)(x + centerX) / BLOCK_WIDTH;
    int blocY = (int)(z + centerY) / BLOCK_WIDTH;

    return (this->getGroundLevel(blocY * BLOCK_PER_MAP_SIDE + blocX, x, z));
}

void RSArea::parseTERA(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["VERS"] = std::bind(&RSArea::parseTERA_VERS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["INFO"] = std::bind(&RSArea::parseTERA_INFO, this, std::placeholders::_1, std::placeholders::_2);
    
    lexer.InitFromRAM(data, size, handlers);
}

void RSArea::parseTERA_VERS(uint8_t *data, size_t size) {}

void RSArea::parseTERA_INFO(uint8_t *data, size_t size) {}

void RSArea::parseTERA_BLOX(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["ELEV"] = std::bind(&RSArea::parseTERA_BLOX_ELEV, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ATRI"] = std::bind(&RSArea::parseTERA_BLOX_ATRI, this, std::placeholders::_1, std::placeholders::_2);
    handlers["OBJS"] = std::bind(&RSArea::parseTERA_BLOX_OBJS, this, std::placeholders::_1, std::placeholders::_2);
    
    lexer.InitFromRAM(data, size, handlers);
}

void RSArea::parseTERA_BLOX_ELEV(uint8_t *data, size_t size) {
    printf("Content of elevation chunk:\n");
    size_t numEleRecords = size / 46;
    ByteStream elevStream(data, size);
    for (size_t e = 0; e < numEleRecords; e++) {

        uint8_t unknownsElev[20];
        for (int i = 0; i < 20; i++)
            unknownsElev[i] = elevStream.ReadByte();

        char elevName[14];
        for (int i = 0; i < 13; i++)
            elevName[i] = elevStream.ReadByte();
        elevName[13] = 0;

        char elevOtherName[14];
        for (int i = 0; i < 13; i++)
            elevOtherName[i] = elevStream.ReadByte();
        elevOtherName[13] = 0;
    }

}

void RSArea::parseTERA_BLOX_ATRI(uint8_t *data, size_t size) {
    ByteStream triStream(data, size);

    char triFileName[13];
    for (int i = 0; i < 13; i++)
        triFileName[i] = triStream.ReadByte();
}

void RSArea::parseTERA_BLOX_OBJS(uint8_t *data, size_t size) {}

void RSArea::parseTERA_TXML(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::unordered_map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSArea::parseTERA_TXML_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MAPS"] = std::bind(&RSArea::parseTERA_TXMS_MAPS, this, std::placeholders::_1, std::placeholders::_2);
    
    lexer.InitFromRAM(data, size, handlers);
}

void RSArea::parseTERA_TXML_INFO(uint8_t *data, size_t size) {}

void RSArea::parseTERA_TXMS_MAPS(uint8_t *data, size_t size) {
    ByteStream textureRefStrean(data, size);
    size_t numTexturesSets = size / 12;
    for (size_t i = 0; i < numTexturesSets; i++) {
        uint16_t fastID = textureRefStrean.ReadUShort();
        char setName[9];
        for (int n = 0; n < 8; n++) {
            setName[n] = textureRefStrean.ReadByte();
        }
        uint8_t unknown = textureRefStrean.ReadByte();
        uint8_t numImages = textureRefStrean.ReadByte();

        printf("Texture Set Ref [%3zu] 0x0x%X[%-8s] %02X (%2u files).\n", i, fastID, setName, unknown, numImages);
    }
}
void RSArea::generateClouds(int count, float altitude, float spread) {
    clouds.clear();

    int m_width = BLOCK_PER_MAP_SIDE * BLOCK_WIDTH;
    int halfWidth = m_width / 2;
    for (int i = 0; i < count; ++i) {
        Cloud c;
        c.position.x = randInt(m_width) - halfWidth;
        c.position.y = altitude + randInt(10000) - 10000;
        c.position.z = randInt(m_width) - halfWidth;
        c.alpha = randFloat(0.70f, 0.95f);

        float cloudRadius = randFloat(1200.0f, 7200.0f);

        int puffCount = 7 + randInt(5);
        for (int p = 0; p < puffCount; ++p) {
            CloudPuff puff;
            float angle = randFloat(0.0f, 6.284f);
            float dist  = randFloat(0.0f, 1.0f);
            dist = dist * dist;

            puff.ox = cosf(angle) * dist * cloudRadius;
            puff.oz = sinf(angle) * dist * cloudRadius;

            float centerFactor = 1.0f - dist;
            puff.oy = centerFactor * centerFactor * cloudRadius * 0.55f;

            float puffR = cloudRadius * (0.38f + centerFactor * 0.42f);
            puffR *= randFloat(0.75f, 1.25f);

            float stretch = randFloat(0.75f, 1.25f);
            puff.rx = puffR * stretch;
            puff.rz = puffR / stretch;
            puff.ry = puffR * randFloat(0.28f, 0.43f);

            c.puffs.push_back(puff);
        }
        clouds.push_back(c);
    }
}
void RSArea::BuildSkirts() {
    skirts_.tris.clear();
    if (BLOCKS_PER_MAP == 0) return;

    const int lod = BLOCK_LOD_MAX;          // doit correspondre au LOD utilisé pour rendre les blocs de bord
    const int N   = BLOCK_PER_MAP_SIDE;
    const int s0  = blocks[lod][0].sideSize;

    // Bord du monde (doit rester cohérent avec renderWorldSkyAndGround)
    const float worldHalf = (BLOCK_WIDTH * BLOCK_PER_MAP_SIDE) * 2.0f;
    const float worldMinX = -worldHalf, worldMaxX = worldHalf;
    const float worldMinZ = -worldHalf, worldMaxZ = worldHalf;

    enum class Side { Top, Right, Bottom, Left };

    struct RingVert {
        float ix, iy, iz;   // inner = sommet EXACT du terrain (aucun décalage)
        float ox, oy, oz;   // outer = projection sur le bord du monde
        float r, g, b;      // couleur EXACTE du sommet du terrain
    };
    std::vector<RingVert> ring;
    ring.reserve(N * s0 * 4);

    // inner = sommet exact ; outer = même point projeté sur le bord du monde du côté donné
    auto makeRing = [&](MapVertex* p, Side s) -> RingVert {
        RingVert v;
        v.ix = p->v.x; v.iy = p->v.y; v.iz = p->v.z;
        v.ox = p->v.x; v.oy = 0; v.oz = p->v.z;   // oy = iy : jupe horizontale à la hauteur du bord
        switch (s) {
            case Side::Top:    v.oz = worldMinZ; break;
            case Side::Bottom: v.oz = worldMaxZ; break;
            case Side::Left:   v.ox = worldMinX; break;
            case Side::Right:  v.ox = worldMaxX; break;
        }
        v.r = p->color[0]; v.g = p->color[1]; v.b = p->color[2];
        return v;
    };
    // Coin : inner = sommet exact ; outer = coin du monde
    auto makeCorner = [&](MapVertex* p, float ox, float oz) -> RingVert {
        RingVert v;
        v.ix = p->v.x; v.iy = p->v.y; v.iz = p->v.z;
        v.ox = ox;     v.oy = 0; v.oz = oz;
        v.r = p->color[0]; v.g = p->color[1]; v.b = p->color[2];
        return v;
    };

    // --- Parcours horaire du bord : chaque sommet UNE seule fois. ---
    // Les 4 coins sont insérés explicitement et donc exclus des boucles de bord.

    // TOP : blocs (row 0), x = 0..s-1  (hors coins TL et TR)
    for (int bx = 0; bx < N; ++bx) {
        AreaBlock* blk = &blocks[lod][bx];
        const int s = blk->sideSize;
        for (int x = 0; x < s; ++x) {
            if (bx == 0     && x == 0)     continue;   // coin TL (inséré en dernier)
            if (bx == N - 1 && x == s - 1) break;      // coin TR (inséré juste après)
            ring.push_back(makeRing(blk->GetVertice(x, 0), Side::Top));
        }
    }
    // Coin TR
    {
        AreaBlock* blk = &blocks[lod][N - 1];
        ring.push_back(makeCorner(blk->GetVertice(blk->sideSize - 1, 0), worldMaxX, worldMinZ));
    }
    // RIGHT : blocs (col N-1), y = 0..s-1  (hors coins TR et BR)
    for (int by = 0; by < N; ++by) {
        AreaBlock* blk = &blocks[lod][by * N + (N - 1)];
        const int s = blk->sideSize;
        for (int y = 0; y < s; ++y) {
            if (by == 0     && y == 0)     continue;   // = coin TR
            if (by == N - 1 && y == s - 1) break;      // coin BR (inséré juste après)
            ring.push_back(makeRing(blk->GetVertice(s - 1, y), Side::Right));
        }
    }
    // Coin BR
    {
        AreaBlock* blk = &blocks[lod][(N - 1) * N + (N - 1)];
        const int s = blk->sideSize;
        ring.push_back(makeCorner(blk->GetVertice(s - 1, s - 1), worldMaxX, worldMaxZ));
    }
    // BOTTOM : blocs (row N-1), x = s-1..0  (hors coins BR et BL)
    for (int bx = N - 1; bx >= 0; --bx) {
        AreaBlock* blk = &blocks[lod][(N - 1) * N + bx];
        const int s = blk->sideSize;
        for (int x = s - 1; x >= 0; --x) {
            if (bx == N - 1 && x == s - 1) continue;   // = coin BR
            if (bx == 0     && x == 0)     break;      // coin BL (inséré juste après)
            ring.push_back(makeRing(blk->GetVertice(x, s - 1), Side::Bottom));
        }
    }
    // Coin BL
    {
        AreaBlock* blk = &blocks[lod][(N - 1) * N + 0];
        ring.push_back(makeCorner(blk->GetVertice(0, blk->sideSize - 1), worldMinX, worldMaxZ));
    }
    // LEFT : blocs (col 0), y = s-1..0  (hors coins BL et TL)
    for (int by = N - 1; by >= 0; --by) {
        AreaBlock* blk = &blocks[lod][by * N + 0];
        const int s = blk->sideSize;
        for (int y = s - 1; y >= 0; --y) {
            if (by == N - 1 && y == s - 1) continue;   // = coin BL
            if (by == 0     && y == 0)     break;      // coin TL (inséré juste après)
            ring.push_back(makeRing(blk->GetVertice(0, y), Side::Left));
        }
    }
    // Coin TL (ferme l'anneau)
    {
        AreaBlock* blk = &blocks[lod][0];
        ring.push_back(makeCorner(blk->GetVertice(0, 0), worldMinX, worldMinZ));
    }

    const size_t M = ring.size();
    if (M < 4) return;
    float avgR = 0, avgG = 0, avgB = 0;
    for (const auto& v : ring) { 
        avgR += v.r;
        avgG += v.g;
        avgB += v.b;
    }
    avgR /= M;
    avgG /= M; 
    avgB /= M;

    const int W = 8; // demi-largeur du filtre, à ajuster
    std::vector<float> sr(M), sg(M), sb(M);
    for (size_t i = 0; i < M; ++i) {
        float sumR = 0;
        float sumG = 0;
        float sumB = 0;
        for (int k = -W; k <= W; ++k) {
            const RingVert& nb = ring[(i + k + M) % M];
            sumR += nb.r;
            sumG += nb.g;
            sumB += nb.b;
        }
        float n = (float)(2 * W + 1);
        sr[i] = sumR / n;
        sg[i] = sumG / n;
        sb[i] = sumB / n;
    }

    // --- Triangulation de la bande (quad strip fermé) ---
    // Inner = sommets EXACTS du terrain => arête interne confondue avec le bord => étanche.
    skirts_.tris.reserve(M * 2);
    for (size_t i = 0; i < M; ++i) {
        const RingVert& A = ring[i];
        const RingVert& B = ring[(i + 1) % M];
        const size_t j = (i + 1) % M;
        skirts_.tris.push_back({{
            {A.ix, A.iy, A.iz, sr[i], sg[i], sb[i]},
            {B.ix, B.iy, B.iz, sr[j], sg[j], sb[j]},
            {B.ox, B.oy, B.oz, avgR,  avgG,  avgB}
        }});
        skirts_.tris.push_back({{
            {A.ix, A.iy, A.iz, sr[i], sg[i], sb[i]},
            {B.ox, B.oy, B.oz, avgR,  avgG,  avgB},
            {A.ox, A.oy, A.oz, avgR,  avgG,  avgB}
        }});
    }
}