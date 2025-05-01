#include "RSObjviewer.h"
#include <algorithm>

RSObjViewer::RSObjViewer()
{
}

RSObjViewer::~RSObjViewer() {
    showCases.clear();
    showCases.shrink_to_fit();
}

void RSObjViewer::initFromFileData(FileData *f) {
    this->initFromRam(f->data, f->size);
}

void RSObjViewer::initFromRam(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["VIEW"] = std::bind(&RSObjViewer::parseVIEW, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSObjViewer::parseVIEW(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["OBJS"] = std::bind(&RSObjViewer::parseVIEW_OBJS, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSObjViewer::parseVIEW_OBJS(uint8_t *data, size_t size) {
    ByteStream stream(data);

    size_t numObjectInList = size / 33;

    for (int objIndex = 0; objIndex < numObjectInList; objIndex++) {

        RSShowCase showCase;

        showCase.filename = stream.ReadString(9);
        showCase.filename.shrink_to_fit();

        std::transform(showCase.filename.begin(), showCase.filename.end(), showCase.filename.begin(), ::toupper);
        std::string model_path = Assets.object_root_path + showCase.filename +".IFF";
        
        TreEntry *entry = Assets.GetEntryByName(model_path);

        if (entry == NULL) {
            printf("Object reference '%s' not found in TRE.\n", model_path.c_str());
            continue;
        }

        showCase.entity = new RSEntity(&Assets);
        showCase.entity->InitFromRAM(entry->data, entry->size);

        showCase.displayName = stream.ReadString(20);
        showCase.displayName.shrink_to_fit();
        uint32_t fixedPointDist = stream.ReadInt32LE();
        showCase.cameraDist = (fixedPointDist >> 8) + (fixedPointDist & 0xFF) / 255.0f;
        // showCase.cameraDist = 200000;
        showCases.push_back(showCase);
    }
}
