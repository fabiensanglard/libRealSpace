//
//  RSImageSet.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 2/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSImageSet.h"

RSImageSet::RSImageSet() {}

RSImageSet::~RSImageSet() {}

void RSImageSet::InitFromPakEntry(PakEntry *entry) {

    uint8_t *end = entry->data + entry->size;
    ByteStream index(entry->data);

    uint32_t nextImage = index.ReadUInt32LE();
    nextImage = nextImage & 0x00FFFFFF;

    uint32_t numImages = nextImage / 4;
    for (size_t i = 0; i < numImages && (entry->data + nextImage < end); i++) {

        uint8_t *currImage = entry->data + nextImage;

        nextImage = index.ReadUInt32LE();
        nextImage = nextImage & 0x00FFFFFF;

        size_t size = 0;

        if (*currImage != 'F') {
            RLEShape *shape = new RLEShape();
            shape->Init(currImage, size);
            this->shapes.push_back(shape);
            this->sequence.push_back((uint8_t)i);
        } else {
            RSPalette *palette = new RSPalette();
            IffLexer paliff;
            uint32_t pal_size = 0;
            pal_size |= *(currImage+4) << 24;
            pal_size |= *(currImage+5) << 16;
            pal_size |= *(currImage+6) << 8;
            pal_size |= *(currImage+7) << 0;
            if (*(currImage+8)=='P') {
                paliff.InitFromRAM(currImage, pal_size+8);
                palette->InitFromIFF(&paliff);
                
                this->palettes.push_back(palette);
                
                RLEShape *shape = new RLEShape();
                *shape = *RLEShape::GetEmptyShape();
                this->shapes.push_back(shape);
            }
        }
    }
}
void RSImageSet::InitFromRam(uint8_t *data, size_t size) {

    uint8_t *end = data + size;
    ByteStream index(data);

    uint32_t nextImage = index.ReadUInt32LE();
    nextImage = nextImage & 0x00FFFFFF;

    uint32_t numImages = nextImage / 4;
    for (size_t i = 0; i < numImages && (data + nextImage < end); i++) {

        uint8_t *currImage = data + nextImage;

        nextImage = index.ReadUInt32LE();
        nextImage = nextImage & 0x00FFFFFF;

        size_t size = 0;

        if (*currImage != 'F') {
            RLEShape *shape = new RLEShape();
            shape->Init(currImage, size);
            this->shapes.push_back(shape);
            this->sequence.push_back((uint8_t)i);
        } else {
            RSPalette *palette = new RSPalette();
            IffLexer paliff;
            uint32_t pal_size = 0;
            pal_size |= *(currImage+4) << 24;
            pal_size |= *(currImage+5) << 16;
            pal_size |= *(currImage+6) << 8;
            pal_size |= *(currImage+7) << 0;
            if (*(currImage+8)=='P') {
                paliff.InitFromRAM(currImage, pal_size+8);
                palette->InitFromIFF(&paliff);
                
                this->palettes.push_back(palette);
                
                RLEShape *shape = new RLEShape();
                *shape = *RLEShape::GetEmptyShape();
                this->shapes.push_back(shape);
            }
        }
    }
}

void RSImageSet::InitFromTreEntry(TreEntry *entry) {

    uint8_t *end = entry->data + entry->size;
    ByteStream index(entry->data);

    uint32_t nextImage = index.ReadUInt32LE();
    nextImage = nextImage & 0x00FFFFFF;

    uint32_t numImages = nextImage / 4;
    for (size_t i = 0; i < numImages && (entry->data + nextImage < end); i++) {

        uint8_t *currImage = entry->data + nextImage;

        nextImage = index.ReadUInt32LE();
        nextImage = nextImage & 0x00FFFFFF;

        size_t size = 0;

        if (*currImage != 'F') {
            RLEShape *shape = new RLEShape();
            shape->Init(currImage, size);
            this->shapes.push_back(shape);
            this->sequence.push_back((uint8_t)i);
        } else {
            RSPalette *palette = new RSPalette();
            IffLexer paliff;
            uint32_t pal_size = 0;
            pal_size |= *(currImage+4) << 24;
            pal_size |= *(currImage+5) << 16;
            pal_size |= *(currImage+6) << 8;
            pal_size |= *(currImage+7) << 0;
            if (*(currImage+8)=='P') {
                paliff.InitFromRAM(currImage, pal_size+8);
                palette->InitFromIFF(&paliff);
                
                this->palettes.push_back(palette);
                
                RLEShape *shape = new RLEShape();
                *shape = *RLEShape::GetEmptyShape();
                this->shapes.push_back(shape);
            }
        }
    }
}

void RSImageSet::InitFromSubPakEntry(PakArchive *entry) {
    for (int i = 0; i < entry->GetNumEntries(); i++) {
        RLEShape *shape = new RLEShape();
        shape->Init(entry->GetEntry(i)->data, 0);
        Point2D pos = {0, 0};
        shape->SetPosition(&pos);
        this->shapes.push_back(shape);
        this->sequence.push_back(i);
    }
}

void RSImageSet::InitFromPakArchive(PakArchive *entry) {
    this->InitFromPakArchive(entry, 8);
}

void RSImageSet::InitFromPakArchive(PakArchive *entry, uint8_t data_offset) {
    for (int i = 0; i < entry->GetNumEntries(); i++) {
        RLEShape *shape = new RLEShape();
        shape->Init(entry->GetEntry(i)->data + data_offset, 0);
        Point2D pos = {0, 0};
        shape->SetPosition(&pos);
        this->shapes.push_back(shape);
        this->sequence.push_back(i);
    }
}


RLEShape *RSImageSet::GetShape(size_t index) { return this->shapes[index]; }

void RSImageSet::Add(RLEShape *shape) { this->shapes.push_back(shape); }

size_t RSImageSet::GetNumImages(void) { return this->shapes.size(); }