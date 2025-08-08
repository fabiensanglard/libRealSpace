#pragma once
#include "../commons/IFFSaxLexer.h"
#include "AssetManager.h"
#include "RSEntity.h"

typedef struct RSShowCase{
        
    float cameraDist;
    RSEntity* entity;
    std::string filename;
    std::string displayName;
    
} RSShowCase;

extern AssetManager Assets;

class RSObjViewer {
public:
    RSObjViewer();
    ~RSObjViewer();
    void initFromFileData(FileData *f);
    void initFromRam(uint8_t *data, size_t size);
    std::vector<RSShowCase> showCases;
private:
    void parseVIEW(uint8_t *data, size_t size);
    void parseVIEW_OBJS(uint8_t *data, size_t size);
};