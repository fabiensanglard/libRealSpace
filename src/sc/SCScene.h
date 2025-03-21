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
    SCZone *createZone(SPRT *sprts, uint8_t sprtId);
    int fps{12};
    bool music{true};

public:
    SCEN *sceneOpts;
    SCScene(PakArchive *optShps, PakArchive *optPals);
    ~SCScene();
    void toggleMusic() { music = !music; };
    void stopMusic() { music = false; };
    virtual std::vector<SCZone *> *
    Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
    virtual void Render();
};

class WeaponLoadoutScene : public SCScene {
    std::vector<SCZone *> extra_zones;
    std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick;
    SCZone *addExtraZone(uint8_t id);
    SCZone *addNoGfXZone(uint8_t id, std::string label, uint8_t weapon_type_shp_id);
    void addWeapon(std::vector<EFCT *> *script, uint8_t id);
    void removeWeapon(std::vector<EFCT *> *script, uint8_t id);
    void updateWeaponDisplay();

    std::vector<std::map<weapon_type_shp_id, std::vector<std::vector<uint8_t>>>> weapon_pylones = {
        std::map<weapon_type_shp_id, std::vector<std::vector<uint8_t>>>{
                                                                        {AIM9J, std::vector<std::vector<uint8_t>>{{4}, {117, 118}}},
                                                                        {AIM9M, std::vector<std::vector<uint8_t>>{{6}, {117, 118}}},
                                                                        },
        std::map<weapon_type_shp_id, std::vector<std::vector<uint8_t>>>{
                                                                        {AIM9J, std::vector<std::vector<uint8_t>>{{5}, {119, 120}}},
                                                                        {AIM9M, std::vector<std::vector<uint8_t>>{{7}, {119, 120}}},
                                                                        {AIM120, std::vector<std::vector<uint8_t>>{{1}, {119, 120}}},
                                                                        },
        std::map<weapon_type_shp_id, std::vector<std::vector<uint8_t>>>{
                                                                        {AIM9J, std::vector<std::vector<uint8_t>>{{40}, {121, 122}}},
                                                                        {AIM9M, std::vector<std::vector<uint8_t>>{{41}, {121, 122}}},
                                                                        {AIM120, std::vector<std::vector<uint8_t>>{{2, 3}, {121, 122}}},
                                                                        {DURANDAL, std::vector<std::vector<uint8_t>>{{14, 15, 16}, {121, 122}}},
                                                                        {MK20, std::vector<std::vector<uint8_t>>{{20, 21, 22}, {121, 122}}},
                                                                        {AGM65D, std::vector<std::vector<uint8_t>>{{8, 9, 10}, {121, 122}}},
                                                                        {MK82, std::vector<std::vector<uint8_t>>{{26, 27, 28}, {121, 122}}},
                                                                        {GBU15, std::vector<std::vector<uint8_t>>{{32}, {121, 122}}},
                                                                        {LAU3, std::vector<std::vector<uint8_t>>{{34, 35, 36}, {121, 122}}},
                                                                        },
        std::map<weapon_type_shp_id, std::vector<std::vector<uint8_t>>>{
                                                                        {DURANDAL, std::vector<std::vector<uint8_t>>{{17, 18, 19}, {123, 124}}},
                                                                        {MK20, std::vector<std::vector<uint8_t>>{{23, 24, 25}, {123, 124}}},
                                                                        {AGM65D, std::vector<std::vector<uint8_t>>{{11, 12, 13}, {123, 124}}},
                                                                        {MK82, std::vector<std::vector<uint8_t>>{{29, 30, 31}, {123, 124}}},
                                                                        {GBU15, std::vector<std::vector<uint8_t>>{{33}, {123, 124}}},
                                                                        {LAU3, std::vector<std::vector<uint8_t>>{{37, 38, 39}, {123, 124}}},
                                                                        }
    };

public:
    void Render();
    WeaponLoadoutScene(PakArchive *optShps, PakArchive *optPals) : SCScene(optShps, optPals) {};
    std::vector<SCZone *> *
    Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
};

class LedgerScene : public SCScene {
    RSFont *font;
    int page{0};
    std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick;

public:
    LedgerScene(PakArchive *optShps, PakArchive *optPals) : SCScene(optShps, optPals) {};
    std::vector<SCZone *> *
    Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
    void CreateZones();
    void Render();
    void TurnPage(std::vector<EFCT *> *script, uint8_t sprite_id) {
        page = (page + 1) % 2;
        this->CreateZones();
    };
};

class CatalogueScene : public SCScene {
public:
    CatalogueScene(PakArchive *optShps, PakArchive *optPals) : SCScene(optShps, optPals) {};
    std::vector<SCZone *> *
    Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
};

class KillBoardScene : public SCScene {
    RSFont *font;
    std::map<uint8_t, std::string> pilot_names = {
        {1, "STERN"},
        {2, "PHOENIX"},
        {3, "ZORRO"},
        {4, "BASELINE"},
        {5, "VIXEN"},
        {6, "PRIME TIME"},
        {7, "TEX"}
    };
public:
    KillBoardScene(PakArchive *optShps, PakArchive *optPals) : SCScene(optShps, optPals) {};
    std::vector<SCZone *> *
    Init(GAMEFLOW_SCEN *gf, SCEN *sc_opts, std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick);
    void Render();
};