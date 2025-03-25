//
//  ConAssetManager.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/1/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

ConvAssetManager::ConvAssetManager() {}

ConvAssetManager::~ConvAssetManager() { Game.Log("We are not freeing the RAM from all the RLEs !!!\n"); }

void ConvAssetManager::Init(void) { BuildDB(); }

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
        Game.Log("ConvAssetManager: Cannot find npc '%s', returning dummy npc instead.\n", name);
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
        Game.Log("ConvAssetManager: Cannot find loc '%s', returning dummy loc instead.\n", name);
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
        Game.Log("ConvAssetManager: Cannot find npc '%s', returning dummy npc instead.\n", name);
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
        Game.Log("Error on Pak %d for layer %d in loc %8s => Using dummy instead\n", shapeID, layerID, back->name);

        // Using an empty shape for now...
        *s = *RLEShape::GetEmptyShape();
        return;
    } else {
        s->Init(subPAK.GetEntry(0)->data, subPAK.GetEntry(0)->size);
        if (s->GetHeight() < 199) {                     //  If this is not a background, we need to move down
            Point2D pos = {0, CONV_TOP_BAR_HEIGHT + 1}; //  to allow the black band on top of the screen
            s->SetPosition(&pos);
        }
    }

    back->layers.push_back(s);
    back->palettes.push_back(paletteArchive->GetEntry(paletteID)->data);
}

/**
 * @brief ReadBackGrounds
 * @param chunkRoot The IffChunk that represent the root of the FORM to parse.
 *
 * This function is used to parse a FORM ILBM and extract the background
 * information. In particular, it reads the name of the background and the
 * layers that composes it. The layers are stored in a specific format that
 * is specific to the Wing Commander game.
 *
 * The format of the layer entry is as follow:
 *  - The first byte is the index of the shape in the CONVSHPS.PAK
 *  - The second byte is the index of the palette in the CONVPALETTES.PAK
 *  - The third byte is the X position of the shape in the background
 *  - The fourth byte is the Y position of the shape in the background
 *  - The fifth byte is the priority of the shape in the background
 *
 * The function also checks that the chunk is a 'FORM' chunk and that the
 * first child of the chunk is an 'INFO' chunk. If not, it terminate the
 * game.
 */
void ConvAssetManager::ReadBackGrounds(const IffChunk *chunkRoot) {

    for (size_t i = 0; i < chunkRoot->childs.size(); i++) {
        IffChunk *chunk = chunkRoot->childs[i];
        if (chunk->id != 'FORM') {
            Game.Log("ConvAssetManager::ReadBackGrounds => Unexpected chunk (%s).\n", chunk->GetName());
            Game.Terminate("Unable to build CONV database.\n");
        }

        IffChunk *info = chunk->childs[0];

        ConvBackGround *back = new ConvBackGround();

        // Get the name
        memset(back->name, 0, 9);
        memcpy(back->name, info->data, info->size);

        // Parse layers and associated bgs.
        size_t numLayers = chunk->childs[1]->size / 5; // A layer entry is 5 bytes wide
        for (size_t layerID = 0; layerID < numLayers; layerID++)
            ParseBGLayer(chunk->childs[1]->data, layerID, back);

        this->backgrounds[back->name] = back;
        // Game.Log("  Able to reach shape in CONVSHPS.PAK entry %d from background '%s'.\n",shapeID,back->name);
    }
}

/**
 * @brief Read a FORM chunk containing Face information.
 *
 * @param[in] root The root chunk of the FORM
 */
void ConvAssetManager::ReadFaces(const IffChunk *root) {
    for (size_t i = 0; i < root->childs.size(); i++) {
        IffChunk *chunk = root->childs[i];
        ByteStream s(chunk->data);

        CharFace *face = new CharFace();
        memcpy(face->name, chunk->data, 8);
        face->name[8] = '\0';

        s.MoveForward(8);

        uint8_t pakID = s.ReadByte();

        RSImageSet *imageSet = new RSImageSet();
        imageSet->InitFromPakEntry(convShps.GetEntry(pakID));
        face->appearances = imageSet;
        for (size_t fid = 0; fid < imageSet->GetNumImages(); fid++) {
            RLEShape *s = imageSet->GetShape(fid);

            int height = s->GetHeight();
            if (height < 198) {
                Point2D pos = {0, CONV_TOP_BAR_HEIGHT + 1}; //  to allow the black band on top of the screen
                s->SetPosition(&pos);
            }
        }

        // printf("Face '%s' features %lu images.\n",face->name,imageSet->GetNumImages());

        this->faces[face->name] = face;
    }
}


/**
 * @brief Read a FORM chunk containing Figure information.
 *
 * @param[in] root The root chunk of the FORM
 */
void ConvAssetManager::ReadFigures(const IffChunk *root) {
    for (size_t i = 0; i < root->childs.size(); i++) {
        IffChunk *chunk = root->childs[i];
        ByteStream s(chunk->data);

        CharFigure *figr = new CharFigure();
        memcpy(figr->name, chunk->data, 8);
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
}


/**
 * @brief Read a FORM chunk containing Palette information for Figures.
 *
 * @param[in] root The root chunk of the FORM
 */
void ConvAssetManager::ReadPFigures(const IffChunk *root) {
    for (size_t i = 0; i < root->childs.size(); i++) {
        IffChunk *chunk = root->childs[i];
        ByteStream s(chunk->data);
        char *figr = new char[8];
        memcpy(figr, chunk->data, 8);
        if (this->figures.find(figr) != this->figures.end()) {
            this->figures[figr]->paletteID = *(chunk->data + 8);
        }
    }
}


/**
 * @brief Read a FORM chunk containing Face Palettes.
 *
 * @param[in] root The root chunk of the FORM
 */
void ConvAssetManager::ReadFCPL(const IffChunk *root) {
    for (size_t i = 0; i < root->childs.size(); i++) {
        FacePalette *pal = new FacePalette();
        memcpy(pal->name, root->childs[i]->data, 8);
        pal->name[8] = '\0';
        pal->index = *(root->childs[i]->data + 8);
        this->facePalettes[pal->name] = pal;
    }
}

// FGPL I have no idea what is in there.
void ConvAssetManager::ReadFGPL(const IffChunk *root) {}

/**
 * @brief Build the database of assets used by the conversation system.
 *
 * This method reads the necessary PAK and IFF files from the TRE archives and
 * initializes the various data structures used by the conversation system.
 */
void ConvAssetManager::BuildDB(void) {

    // This is were the background shapes are stored.
    TreEntry *convShapEntry =
        Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVSHPS.PAK");
    convShps.InitFromRAM("CONVSHPS.PAK", convShapEntry->data, convShapEntry->size);
    // convShapeArchive.List(stdout);

    // This is were the palette patches are stored
    TreEntry *convPalettesEntry =
        Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVPALS.PAK");
    convPals.InitFromRAM("CONVPALS.PAK", convPalettesEntry->data, convPalettesEntry->size);
    // convPalettePak.List(stdout);

    // This is were the background shapes are stored.
    TreEntry *optShapEntry =
        Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    // optShps(stdout);

    // This is were the palette patches are stored
    TreEntry *optPalettesEntry =
        Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
    optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
    // optPals(stdout);

    // Open the metadata
    TreEntry *convDataEntry =
        Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVDATA.IFF");
    IffLexer convDataLexer;
    convDataLexer.InitFromRAM(convDataEntry->data, convDataEntry->size);
    // convDataLexer.List(stdout);

    ReadBackGrounds(convDataLexer.GetChunkByID('BCKS'));

    ReadFaces(convDataLexer.GetChunkByID('FACE')); // PAK id for Face image collection

    ReadFigures(convDataLexer.GetChunkByID('FIGR')); // PAK id for Figures image

    ReadPFigures(convDataLexer.GetChunkByID('PFIG')); // ??!? Maybe Palette figure ???!?!

    // I have no idea what is in there.
    ReadFCPL(convDataLexer.GetChunkByID('FCPL')); // Face Conv Palette normal and night

    // I have no idea what is in there.
    ReadFGPL(convDataLexer.GetChunkByID('FGPL')); // Face Game palette normal
}