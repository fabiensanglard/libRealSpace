#pragma once
#include "precomp.h"
/**
 * background is a structure used to store information about a background.
 *
 * - img: a pointer to a RSImageSet, which is a collection of images that can be used for animation.
 * - frameCounter: a variable that keeps track of the current frame of the animation.
 */
struct background {
    RSImageSet *img{nullptr};
    uint8_t frameCounter{0};
};
class SCScene {
protected:
    std::vector<EFCT *> *efect;
    std::vector<SCZone *> zones;
    GAMEFLOW_SCEN *gameflow_scene;
    
    std::vector<background *> layers;
    uint8_t *rawPalette;
    uint8_t *forPalette;
    VGAPalette palette;
    PakArchive *optShps;
    PakArchive *optPals;
    RSImageSet *getShape(uint8_t shpid);
    int fps{12};
public:
    SCEN *sceneOpts;
    SCScene(PakArchive *optShps, PakArchive *optPals);
    ~SCScene();
    virtual std::vector<SCZone *> Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
    void Render();
};

class WeaponLoadoutScene : public SCScene {
public:
    WeaponLoadoutScene(PakArchive *optShps, PakArchive *optPals) : SCScene(optShps, optPals) {};
    std::vector<SCZone *> Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
};

class LedgerScene : public SCScene {
public:
    LedgerScene(PakArchive *optShps, PakArchive *optPals) : SCScene(optShps, optPals) {};
    std::vector<SCZone *> Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
};

class CatalogueScene : public SCScene {
public:
    CatalogueScene(PakArchive *optShps, PakArchive *optPals) : SCScene(optShps, optPals) {};
    std::vector<SCZone *> Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
};