//
//  ConAssetManager.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/1/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

ConvAssetManager::ConvAssetManager() {
    this->conv_file_name = "CONVDATA.IFF";
}

ConvAssetManager::~ConvAssetManager() { printf("We are not freeing the RAM from all the RLEs !!!\n"); }

void ConvAssetManager::init(void) { BuildDB(); }

/**
 * Given a name, return the associated CharFace object.
 *
 * If the character is not found, a dummy object is created on the fly and
 * the method returns it. The dummy object is stored in the internal map
 * so that subsequent calls with the same name will return the same
 * dummy object.
 *
 * @param name Name of the character to retrieve.
 *
 * @return A CharFace object.
 */
CharFace *ConvAssetManager::GetCharFace(char *name) {

    CharFace *npc = this->faces[name];

    if (npc == NULL) {
        static CharFace dummy;
        RSImageSet *set = new RSImageSet();
        set->Add(RLEShape::GetEmptyShape());
        dummy.appearances = set;
        npc = &dummy;
        printf("ConvAssetManager: Cannot find npc '%s', returning dummy npc instead.\n", name);
        this->faces[name] = npc;
    }

    return npc;
}

/**
 * Return a ConvBackGround object associated with the given name.
 *
 * If the figure doesn't exist, it creates a dummy one and logs a message
 * to the console.
 *
 * @param name Name of the figure to retrieve.
 *
 * @return A ConvBackGround object.
 */
ConvBackGround *ConvAssetManager::GetBackGround(char *name) {

    ConvBackGround *shape = this->backgrounds[name];

    if (shape == NULL) {
        printf("ConvAssetManager: Cannot find loc '%s', returning dummy loc instead.\n", name);
        static ConvBackGround dummy;
        uint8_t dummyPalettePatch[5] = {0, 0, 0, 0, 0};
        dummy.palettes.push_back(dummyPalettePatch);
        dummy.layers.push_back(RLEShape::GetEmptyShape());
        shape = &dummy;
        this->backgrounds[name] = shape;
    }

    return shape;
}

/**
 * Return a CharFigure object associated with the given name.
 *
 * If the figure doesn't exist, it creates a dummy one and logs a message
 * to the console.
 *
 * @param name Name of the figure to retrieve.
 *
 * @return A CharFigure object.
 */
CharFigure *ConvAssetManager::GetFigure(char *name) {
    if (this->figures.count(name) > 0) {
        return this->figures[name];
    } else {
        CharFigure *npc = NULL;
        static CharFigure dummy;
        RSImageSet *set = new RSImageSet();
        set->Add(RLEShape::GetEmptyShape());
        dummy.appearances = set;
        npc = &dummy;
        printf("ConvAssetManager: Cannot find npc '%s', returning dummy npc instead.\n", name);
        this->figures[name] = npc;
        return npc;
    }
}

/**
 * Given a face name, return the ID of its palette. If the face is not found, return 0.
 */
uint8_t ConvAssetManager::GetFacePaletteID(char *name) {
    if (this->facePalettes.count(name) > 0) {
        return this->facePalettes[name]->index;
    }
    return 0;
}

/**
 * Parse a single layer of a ConvBackGround.
 *
 * \param data The raw data of the layer.
 * \param layerID The ID of the layer to parse.
 * \param back The ConvBackGround object to store the parsed layer in.
 *
 * This function parses a single layer of a ConvBackGround from the raw
 * data.  It reads the type, shape ID, and palette ID from the data, and
 * then looks up the corresponding RLEShape and palette in the
 * appropriate PAK archive.  If the shape or palette is not found, it
 * returns a dummy shape instead.
 *
 */
void ConvAssetManager::ParseBGLayer(uint8_t *data, size_t layerID, ConvBackGround *back) {

    ByteStream dataReader;
    dataReader.Set(data + 5 * layerID);

    uint8_t type = dataReader.ReadByte();
    uint8_t shapeID = dataReader.ReadByte();
    uint8_t paletteID = dataReader.ReadByte();

    PakArchive *shapeArchive = NULL;
    PakArchive *paletteArchive = NULL;

    if (type == 0x00) {
        // RLEShape is in CONVSHPS.PAK and Palette is in CONVPALS.PAK
        shapeArchive = &this->convShps;
        paletteArchive = &this->convPals;
    }

    if (type == 0x01) {
        // RLEShape is in OPTSHPS.PAK and Palette is in OPTPALS.PAK
        shapeArchive = &this->optShps;
        paletteArchive = &this->optPals;
    }

    // Debug Display
    /*
        printf("\n%8s layer %lu :",back->name,layerID);
        for (size_t x=0; x < 5 ; x++) {
            printf("%3d ",*(data + 5 * layerID+x));
        }
    */

    /*
    // The pack features some duplicate entries.
    while(convShapeArchive.GetEntry(shapeID)->size == 0)
        shapeID--;
    */

    RLEShape *s = new RLEShape();

    PakEntry *shapeEntry = shapeArchive->GetEntry(shapeID);
    PakArchive subPAK;
    subPAK.InitFromRAM("", shapeEntry->data, shapeEntry->size);

    if (!subPAK.IsReady()) {

        // Sometimes the image is not in a PAK but as raw data.
        printf("Error on Pak %d for layer %d in loc %8s => Using dummy instead\n", shapeID, layerID, back->name);

        // Using an empty shape for now...
        *s = *RLEShape::GetEmptyShape();
        return;
    } else {
        s->init(subPAK.GetEntry(0)->data, subPAK.GetEntry(0)->size);
        if (s->GetHeight() < 199) {                     //  If this is not a background, we need to move down
            Point2D pos = {0, CONV_TOP_BAR_HEIGHT + 1}; //  to allow the black band on top of the screen
            s->SetPosition(&pos);
        }
    }

    back->layers.push_back(s);
    back->palettes.push_back(paletteArchive->GetEntry(paletteID)->data);
}

/**
 * @brief Build the database of assets used by the conversation system.
 *
 * This method reads the necessary PAK and IFF files from the TRE archives and
 * initializes the various data structures used by the conversation system.
 */
void ConvAssetManager::BuildDB(void) {

    // This is were the background shapes are stored.
    TreEntry *convShapEntry =
        Assets.GetEntryByName(Assets.conv_pak_filename);
    convShps.InitFromRAM("CONVSHPS.PAK", convShapEntry->data, convShapEntry->size);
    // convShapeArchive.List(stdout);

    // This is were the palette patches are stored
    TreEntry *convPalettesEntry =
        Assets.GetEntryByName(Assets.conv_pal_filename);
    convPals.InitFromRAM("CONVPALS.PAK", convPalettesEntry->data, convPalettesEntry->size);
    // convPalettePak.List(stdout);

    // This is were the background shapes are stored.
    TreEntry *optShapEntry =
        Assets.GetEntryByName(Assets.optshps_filename);
    optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    // optShps(stdout);

    // This is were the palette patches are stored
    TreEntry *optPalettesEntry =
        Assets.GetEntryByName(Assets.optpals_filename);
    optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
    // optPals(stdout);

    // Open the metadata
    TreEntry *convDataEntry =
        Assets.GetEntryByName(Assets.conv_data_filename);
    this->parseIFF(convDataEntry->data, convDataEntry->size);
}
void ConvAssetManager::parseIFF(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["BCKS"] = std::bind(&ConvAssetManager::parseBCKS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["FACE"] = std::bind(&ConvAssetManager::parseFACE, this, std::placeholders::_1, std::placeholders::_2);
    handlers["FIGR"] = std::bind(&ConvAssetManager::parseFIGR, this, std::placeholders::_1, std::placeholders::_2);
    handlers["PFIG"] = std::bind(&ConvAssetManager::parsePFIG, this, std::placeholders::_1, std::placeholders::_2);
    handlers["FCPL"] = std::bind(&ConvAssetManager::parseFCPL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["FGPL"] = std::bind(&ConvAssetManager::parseFGPL, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void ConvAssetManager::parseBCKS(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["BACK"] = std::bind(&ConvAssetManager::parseBCKS_BACK, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void ConvAssetManager::parseBCKS_BACK(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    this->tmp_conv_bg = new ConvBackGround();
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&ConvAssetManager::parseBCKS_BACK_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DATA"] = std::bind(&ConvAssetManager::parseBCKS_BACK_DATA, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
    this->backgrounds[this->tmp_conv_bg->name] = this->tmp_conv_bg;

}
void ConvAssetManager::parseBCKS_BACK_INFO(uint8_t *data, size_t size) {
    memset(this->tmp_conv_bg->name, 0, 9);
    memcpy(this->tmp_conv_bg->name, data, size);
}
void ConvAssetManager::parseBCKS_BACK_DATA(uint8_t *data, size_t size) {
    size_t numLayers = size / 5; // A layer entry is 5 bytes wide
    for (size_t layerID = 0; layerID < numLayers; layerID++)
        ParseBGLayer(data, layerID, tmp_conv_bg);
}
void ConvAssetManager::parseFACE(uint8_t *data, size_t size) {

    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["DATA"] = std::bind(&ConvAssetManager::parseFACE_DATA, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void ConvAssetManager::parseFACE_DATA(uint8_t *data, size_t size) {
    ByteStream s(data);

    CharFace *tmp_face = new CharFace();
    memcpy(tmp_face->name, data, 8);
    tmp_face->name[8] = '\0';

    s.MoveForward(8);

    uint8_t pakID = s.ReadByte();

    RSImageSet *imageSet = new RSImageSet();
    imageSet->InitFromPakEntry(convShps.GetEntry(pakID));
    tmp_face->appearances = imageSet;
    for (size_t fid = 0; fid < imageSet->GetNumImages(); fid++) {
        RLEShape *s = imageSet->GetShape(fid);

        int height = s->GetHeight();
        if (height < 198) {
            Point2D pos = {0, CONV_TOP_BAR_HEIGHT + 1}; //  to allow the black band on top of the screen
            s->SetPosition(&pos);
        }
    }

    this->faces[tmp_face->name] = tmp_face;
}
void ConvAssetManager::parseFIGR(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["DATA"] = std::bind(&ConvAssetManager::parseFIGR_DATA, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void ConvAssetManager::parseFIGR_DATA(uint8_t *data, size_t size) {
    ByteStream s(data);

    CharFigure *figr = new CharFigure();
    memcpy(figr->name, data, 8);
    figr->name[8] = '\0';

    s.MoveForward(8);

    uint8_t pakID = s.ReadByte();

    RSImageSet *shape = new RSImageSet();
    PakEntry *entry = convShps.GetEntry(pakID);
    PakArchive *pak = new PakArchive();
    pak->InitFromRAM("FIGR", entry->data, entry->size);
    if (pak->GetNumEntries()>0) {
        shape->InitFromSubPakEntry(pak);
    } else {
        shape->InitFromPakEntry(entry);
    }

    Point2D pos = {0, CONV_TOP_BAR_HEIGHT + 1}; //  to allow the black band on top of the screen
    figr->appearances = shape;
    this->figures[figr->name] = figr;
}
void ConvAssetManager::parsePFIG(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["DATA"] = std::bind(&ConvAssetManager::parsePFIG_DATA, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void ConvAssetManager::parsePFIG_DATA(uint8_t *data, size_t size) {
    ByteStream s(data);
    char *figr = new char[9];
    memcpy(figr, data, 8);
    figr[8] = '\0';
    if (this->figures.find(figr) != this->figures.end()) {
        this->figures[figr]->paletteID = *(data + 8);
    }
}
void ConvAssetManager::parseFCPL(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["DATA"] = std::bind(&ConvAssetManager::parseFCPL_DATA, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void ConvAssetManager::parseFCPL_DATA(uint8_t *data, size_t size) {
    FacePalette *pal = new FacePalette();
    memcpy(pal->name, data, 8);
    pal->name[8] = '\0';
    pal->index = *(data + 8);
    this->facePalettes[pal->name] = pal;
}
void ConvAssetManager::parseFGPL(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["DATA"] = std::bind(&ConvAssetManager::parseFGPL_DATA, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void ConvAssetManager::parseFGPL_DATA(uint8_t *data, size_t size) {
    
}