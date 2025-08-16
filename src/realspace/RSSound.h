#pragma once
#include "precomp.h"
#include <cstdint>
#include <vector>
#include <map>

struct MemSound {
    uint8_t *data;
    size_t size;
    uint8_t id{0};
};
struct InGameVoice {
    std::map<uint8_t, MemSound *> messages;
    uint8_t id{0};
};

class RSSound {
public:
    std::map<uint8_t, InGameVoice *> inGameVoices;
    std::vector<MemSound *> sounds;
    void init(AssetManager *assetManager);
};