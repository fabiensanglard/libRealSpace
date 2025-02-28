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

enum weapon_type_id {
    AIM9J = 29,
    AIM9M = 30, 
    AIM120 = 116,
    AGM65D = 31,
    DURANDAL = 32,
    MK20 = 33,
    MK82 = 34,
    GBU15 = 35,
    LAU3 = 36
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
    SCZone *createZone(SPRT *sprts, uint8_t sprtId);
    int fps{12};
    bool music{true};
public:
    SCEN *sceneOpts;
    SCScene(PakArchive *optShps, PakArchive *optPals);
    ~SCScene();
    void toggleMusic() { music = !music; };
    void stopMusic() { music = false; };
    virtual std::vector<SCZone *> *Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
    virtual void Render();
};

class WeaponLoadoutScene : public SCScene {
    std::vector<SCZone *> extra_zones;
    std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick;
    SCZone *addExtraZone(uint8_t id);
    SCZone *addNoGfXZone(uint8_t id, std::string label, uint8_t weapon_type_id);
    void addWeapon(std::vector<EFCT *> *script, uint8_t id);
    void removeWeapon(std::vector<EFCT *> *script, uint8_t id);
    void updateWeaponDisplay();
public:
    void Render();
    WeaponLoadoutScene(PakArchive *optShps, PakArchive *optPals) : SCScene(optShps, optPals) {};
    std::vector<SCZone *> *Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
};

class LedgerScene : public SCScene {
public:
    LedgerScene(PakArchive *optShps, PakArchive *optPals) : SCScene(optShps, optPals) {};
    std::vector<SCZone *> *Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
};

class CatalogueScene : public SCScene {
public:
    CatalogueScene(PakArchive *optShps, PakArchive *optPals) : SCScene(optShps, optPals) {};
    std::vector<SCZone *> *Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
};