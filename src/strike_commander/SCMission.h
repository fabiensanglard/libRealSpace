#pragma once
#include "precomp.h"

struct SCAiPlane {
    SCPlane *plane{nullptr};
    SCPilot *pilot{nullptr};
    MISN_PART *object{nullptr};
    RSProf *ai{nullptr};
    std::string name;
};

struct SCMissionWaypoint {
    SPOT *spot{nullptr};
    std::string *message{nullptr};
    std::string *objective{nullptr};
};

struct RadioMessages {
    std::string message;
    MemSound *sound{nullptr};
};


class SCProg;
class SCCameraDirector;

class SCMission {
    
private:
    MessageBus::SubscriptionId subscription_id{-1};
protected:
    std::string mission_name;
    std::unordered_map<std::string, RSEntity *> *obj_cache{nullptr};
    
    RSProf * LoadProfile(std::string name);
    uint32_t last_time{0};
    uint32_t last_tick{0};
    uint32_t tick_counter{0};
    // Cadence de decision IA d'origine (~25 fps, cf. analysis/AI_SYSTEM.md
    // §6) : AIRefreshEvent est publie a ce rythme fixe plutot qu'a chaque
    // frame reelle du port, pour ne pas rendre l'IA statistiquement plus
    // forte/reactive sur un framerate superieur a l'origine.
    float ai_refresh_accumulator{0.0f};
    static constexpr float AI_REFRESH_INTERVAL = 1.0f / 25.0f;
    float music_accumulator{0.0f};
    static constexpr float MUSIC_UPDATE_INTERVAL = 16.0f / 25.0f;
    bool music_combat_happened{false};
    uint8_t music_enemy_flags{0};
    int music_resume_tune{-1};
    bool music_end_played{false};
    bool music_eject_requested{false};
    bool music_shot_down_requested{false};
    bool music_landing_requested{false};
    void onMusicUpdate();
    void onActorDestroyed(SCMissionActors *actor);
    void musicPollEvents();
    void musicRequest(int tune, const char *reason);
    uint8_t musicScanEnemies(SCMissionActors *excluded);
    bool musicFallback();
    float playerDamage();
    bool enemyOnPlayerSix();
    
    SCState &GameState = SCState::getInstance();
    AssetManager &Assets = AssetManager::getInstance();
    SCRenderer &Renderer = SCRenderer::getInstance();
    RSMixer &Mixer = RSMixer::getInstance();
    MessageBus &messageBus = MessageBus::getInstance();
    void onEvent(const EventMessage &event);
public:
    std::vector<SCMissionScene *> scenes;
    std::vector<SCMissionActors *> actors;
    std::vector<SCMissionActors *> enemies;
    std::vector<SCMissionActors *> friendlies;
    std::vector<SCMissionWaypoint *> waypoints;
    std::vector<SCExplosion *> explosions;
    std::unordered_map<uint16_t, int16_t> gameflow_registers;
    std::unordered_map<uint8_t, std::vector<uint8_t>> progs_traces;
    bool success{false};
    bool failure{false};
    bool mission_ended{false};
    uint8_t current_area_id{0};
    SCMissionActors *player{nullptr};
    RSArea *area{nullptr};
    RSMission *mission{nullptr};
    RSIntel intel;
    SCMissionActors *player_tail_seen{nullptr};    // word_722EA
    SCMissionActors *player_tail_threat{nullptr};  // word_722EE
    bool aa_missile_launched{false};               // byte_6E4C6
    bool aa_missile_launched_last{false};          // byte_6E4D7
    float ai_clock_stagger{0.0f};                  // dword_6D3BE
    void onWeaponSpawned(SCSimulatedObject *weapon);
    void onWeaponRemoved(SCSimulatedObject *weapon);
    RSWorld *world{nullptr};
    SCCameraDirector *camera_director{nullptr};
    RSSound &sound = RSSound::getInstance();
    bool in_combat{false};
    std::vector<RadioMessages*> radio_messages;
    bool mission_over{false};
    bool mission_won{false};
    SCMission();
    SCMission(std::string mission_name, std::unordered_map<std::string, RSEntity *> *objCache);
    ~SCMission();
    virtual void loadMission();
    void cleanup();
    virtual void update();
    void executeProg(std::vector<PROG> *prog);
    uint8_t getAreaID(Vector3D position);
    RSEntity * LoadEntity(std::string name);
    void applyDifficulty();
    uint32_t tps{0};
    float scene_wait_timer{0.0f};   // dword_706B0 : minuteur WAIT des scripts de scene
    int32_t difficulty{0};
};


