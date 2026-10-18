//
//  SCAnimationArchive.h
//  libRealSpace
//
//  Created on 2/10/2025.
//

#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include <unordered_map>

#include "../commons/IFFSaxLexer.h"
#include "../commons/IFFWriter.h"
#include "PakArchive.h"

constexpr uint32_t MIDGAME_ANIM_VERSION = 2;
constexpr uint8_t MIDGAME_MUSIC_KEEP = 255;
constexpr uint8_t MIDGAME_MUSIC_STOP = 254;
constexpr uint8_t MIDGAME_NO_IMAGE = 255;

typedef struct MIDGAME_LAYER_COMMON {
    int16_t layer_id{-1};
    int16_t parent_id{-1};
    uint16_t frame_start{0};
    uint16_t nb_frames{0};
    std::vector<Point2D> path;
    bool jitter{false};
    bool clip{false};
    Point2D clip_min{};
    Point2D clip_max{};
    int16_t depth{0};
    bool game_numbering{false};
    std::vector<uint8_t> sequence;
    Point2D jitter_offset{};
    int image_offset{0};
} MIDGAME_LAYER_COMMON;

typedef struct MIDGAME_PALETTE_REF {
    std::string archive;
    uint16_t entry{0};
    PakArchive *pak{nullptr};
} MIDGAME_PALETTE_REF;

typedef struct MIDGAME_LIP_TEXT {
    uint16_t frame_start{0};
    std::string text;
} MIDGAME_LIP_TEXT;

typedef struct MIDGAME_SHOT_BG {
    PakArchive *pak;
    RSImageSet *image;
    PakArchive *pak_palette{nullptr};
    uint8_t palette{255};
    RSPalette *pal;
    Point2D position_start;
    Point2D position_end;
    Point2D velocity;
    Point2D current_position{0,0};
    int shapeid;
    int pak_entry_id;
    bool use_external_palette{false};
    std::string face;
    MIDGAME_LAYER_COMMON common;
} MIDGAME_SHOT_BG;

typedef struct MIDGAME_SHOT_CHARACTER {
    std::string character_name;
    std::string background_name;
    Point2D position_start;
    Point2D position_end;
    Point2D velocity;
    RSImageSet *image;
    RSPalette *pal;
    uint8_t palette{255};
    uint8_t cloth_id;
    uint8_t head_id;
    uint8_t expression_id;
    bool talking{false};
    int current_frame{0};
    MIDGAME_LAYER_COMMON common;
    std::vector<MIDGAME_LIP_TEXT> lips;
    Point2D current_position{};
    int lip_entry{-1};
    int lip_index{0};
    int16_t mouth{-1};
} MIDGAME_SHOT_CHARACTER;

typedef struct MIDGAME_TEXT_LINE {
    std::string key;
    int8_t id{-1};
    int8_t font_id{-1};
    uint16_t frame_start{0};
    uint16_t nb_frames{0};
    Point2D position_start;
    Point2D position_end;
    uint8_t color{255};
    uint8_t align{0};
} MIDGAME_TEXT_LINE;

typedef struct MIDGAME_SHAPE_DATA {
    PakArchive *pak;
    uint8_t shape_id;
    uint8_t sub_shape_id;
    uint8_t pal_id;
    Point2D start;
    Point2D end;
    Point2D velocity;
    uint8_t keep_first_frame;
} MIDGAME_SHAPE_DATA;

typedef struct MIDGAME_SHOT_SPRITE {
    PakArchive *pak;
    int pak_entry_id;
    RSImageSet *image;
    RSPalette *pal;
    PakArchive *pak_palette{nullptr};
    uint8_t palette{255};
    Point2D position_start;
    Point2D position_end;
    Point2D velocity;
    Point2D current_position{0,0};
    int16_t start_frame{-1};
    int16_t end_frame{-1};
    int shapeid;
    uint8_t keep_first_frame;
    bool use_external_palette{false};
    int current_frame{1};
    bool repeat_animation{false};
    bool finished{false};
    std::string face;
    MIDGAME_LAYER_COMMON common;
} MIDGAME_SHOT_SPRITE;

typedef struct MIDGAME_SOUND {
    uint8_t *data;
    size_t size;

} MIDGAME_SOUND;

typedef struct MIDGAME_SHOT_SOUND {
    PakArchive *pak{nullptr};
    int pak_entry_id{-1};
    MIDGAME_SOUND *sound{nullptr};
    uint16_t frame_start{0};
    bool played{false};
} MIDGAME_SHOT_SOUND;

typedef struct MIDGAME_SHOT {
    std::vector<MIDGAME_SHOT_BG *>background;
    std::vector<MIDGAME_SHOT_SPRITE *>sprites;
    std::vector<MIDGAME_SHOT_BG *>foreground;
    std::vector<MIDGAME_SHOT_CHARACTER *>characters;
    std::vector<MIDGAME_TEXT_LINE *> textes;
    int nbframe{0};
    int music{255};
    int sound_time_code{0};
    uint8_t fps{0};
    bool wait_sound{false};
    bool keep_palette{false};
    uint8_t fade_in{0};
    int16_t end_layer{-1};
    bool full_screen{false};
    bool clear_palette{false};
    std::vector<MIDGAME_PALETTE_REF> palettes;
    std::vector<MIDGAME_SHOT_SOUND *> sounds;
} MIDGAME_SHOT;


class SCAnimationArchive {
public:
    SCAnimationArchive();
    ~SCAnimationArchive();

    // Enregistrer une animation dans un fichier IFF
    bool SaveToFile(const char* filename, const std::vector<MIDGAME_SHOT*>& shots);
    
    // Enregistrer une animation en mémoire
    bool SaveToMemory(uint8_t** data, size_t* size, const std::vector<MIDGAME_SHOT*>& shots);

    // Charger une animation depuis un fichier IFF
    bool LoadFromFile(const char* filename, std::vector<MIDGAME_SHOT*>& shots);
    
    // Charger une animation depuis la mémoire
    bool LoadFromRAM(uint8_t* data, size_t size, std::vector<MIDGAME_SHOT*>& shots);
    std::vector<MIDGAME_SHOT*>* shots{nullptr};
    uint32_t version{1};
private:
    // Structure pour stocker les références aux PakArchive
    struct PakArchiveRef {
        std::string name;
        PakArchive* archive;
    };

    // Liste des archives utilisées lors du chargement
    std::vector<PakArchiveRef> pakArchives;

    // Variables temporaires utilisées pendant le chargement
    MIDGAME_SHOT* currentShot;
    MIDGAME_SHOT_BG* currentBg;
    MIDGAME_SHOT_SPRITE* currentSprite{nullptr};
    MIDGAME_SHOT_CHARACTER* currentCharacter{nullptr};
    MIDGAME_LAYER_COMMON* currentCommon{nullptr};

    // Méthodes pour l'écriture
    void WriteAnimation(IFFWriter& writer, const std::vector<MIDGAME_SHOT*>& shots);
    void WriteShot(IFFWriter& writer, const MIDGAME_SHOT* shot);
    void WriteBackgrounds(IFFWriter& writer, const std::vector<MIDGAME_SHOT_BG*>& backgrounds);
    void WriteBackground(IFFWriter& writer, const MIDGAME_SHOT_BG* bg);
    void WriteSprites(IFFWriter& writer, const std::vector<MIDGAME_SHOT_SPRITE*>& sprites);
    void WriteSprite(IFFWriter& writer, const MIDGAME_SHOT_SPRITE* sprite);
    void WriteTexts(IFFWriter& writer, const std::vector<MIDGAME_TEXT_LINE *>& textes);
    void WriteForegrounds(IFFWriter& writer, const std::vector<MIDGAME_SHOT_BG*>& foregrounds);
    void WriteSound(IFFWriter& writer, PakArchive* pak, int pak_entry_id, uint16_t frame_start);
    void WriteLayerCommon(IFFWriter& writer, const MIDGAME_LAYER_COMMON& common);
    void WriteSequence(IFFWriter& writer, const std::vector<uint8_t>& sequence);
    void WriteLips(IFFWriter& writer, const std::vector<MIDGAME_LIP_TEXT>& lips);

    // Handler pour l'IFFSaxLexer
    void HandleANIM(uint8_t* data, size_t size);
    void HandleHEAD(uint8_t* data, size_t size);
    void HandleLAYR(uint8_t* data, size_t size);
    void HandlePATH(uint8_t* data, size_t size);
    void HandleSEQU(uint8_t* data, size_t size);
    void HandleLIPS(uint8_t* data, size_t size);
    void HandlePALS(uint8_t* data, size_t size);
    void WritePalettes(IFFWriter& writer, const MIDGAME_SHOT* shot);
    std::string ArchiveName(PakArchive* pak);
    RSImageSet* LoadLayerImage(const std::string& archiveName, PakArchive* pak, int entryId, std::string& face,
                               MIDGAME_LAYER_COMMON& common);
    void HandleSHOT(uint8_t* data, size_t size);
    void HandleSHTP(uint8_t* data, size_t size);
    void HandleBGND(uint8_t* data, size_t size);
    void HandleBKGD(uint8_t* data, size_t size);
    void HandleSPRT(uint8_t* data, size_t size);
    void HandleSPRI(uint8_t* data, size_t size);
    void HandleFGND(uint8_t* data, size_t size);
    void HandleFRGD(uint8_t* data, size_t size);
    void HandleSOND(uint8_t* data, size_t size);
    void WriteCharacters(IFFWriter& writer, const std::vector<MIDGAME_SHOT_CHARACTER*>& characters);
    void HandleCHAR(uint8_t* data, size_t size);
    void HandleCHRC(uint8_t* data, size_t size);
    void HandleTEXT(uint8_t* data, size_t size);
    void HandleTEXT_TEXE(uint8_t* data, size_t size);

    // Utilitaires
    PakArchive* FindOrLoadPakArchive(const std::string& name);
};