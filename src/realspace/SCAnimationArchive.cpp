//
//  SCAnimationArchive.cpp
//  libRealSpace
//
//  Created on 2/10/2025.
//

#include "precomp.h"
#include "SCAnimationArchive.h"
#include "../commons/ByteStream.h"
#include "AssetManager.h"

SCAnimationArchive::SCAnimationArchive() {
}

SCAnimationArchive::~SCAnimationArchive() {
}

bool SCAnimationArchive::SaveToFile(const char* filename, const std::vector<MIDGAME_SHOT*>& shots) {
    IFFWriter writer;
    writer.StartIFF("ANIM");
    WriteAnimation(writer, shots);
    return writer.SaveToFile(filename);
}

bool SCAnimationArchive::SaveToMemory(uint8_t** data, size_t* size, const std::vector<MIDGAME_SHOT*>& shots) {
    IFFWriter writer;
    writer.StartIFF("ANIM");
    WriteAnimation(writer, shots);
    return writer.SaveToMemory(data, size);
}

bool SCAnimationArchive::LoadFromFile(const char* filename, std::vector<MIDGAME_SHOT*>& shots) {
    IFFSaxLexer lexer;
    this->shots = &shots;
    std::unordered_map<std::string, std::function<void(uint8_t*, size_t)>> events;
    events["ANIM"] = std::bind(&SCAnimationArchive::HandleANIM, this, std::placeholders::_1, std::placeholders::_2);
    return lexer.InitFromFile(filename, events);
}

bool SCAnimationArchive::LoadFromRAM(uint8_t* data, size_t size, std::vector<MIDGAME_SHOT*>& shots) {
    IFFSaxLexer lexer;
    
    std::unordered_map<std::string, std::function<void(uint8_t*, size_t)>> events;
    events["ANIM"] = std::bind(&SCAnimationArchive::HandleANIM, this, std::placeholders::_1, std::placeholders::_2);
    
    return lexer.InitFromRAM(data, size, events);
}

void SCAnimationArchive::WriteAnimation(IFFWriter& writer, const std::vector<MIDGAME_SHOT*>& shots) {
    writer.StartChunk("HEAD");
    // En-tête - version du format, nombre de shots, etc.
    writer.WriteUint32(shots.size()); // Nombre de shots
    writer.WriteUint32(MIDGAME_ANIM_VERSION);
    writer.EndChunk();
    
    for (const MIDGAME_SHOT* shot : shots) {
        WriteShot(writer, shot);
    }
}

void SCAnimationArchive::WriteShot(IFFWriter& writer, const MIDGAME_SHOT* shot) {
    writer.StartChunk("SHOT");
    // Propriétés du shot
    writer.StartChunk("SHTP");
    writer.WriteInt32(shot->nbframe);
    writer.WriteUint8(shot->music);
    uint16_t sound_time_code = 0;
    if (!shot->sounds.empty()) {
        sound_time_code = shot->sounds[0]->frame_start + 1;
    }
    writer.WriteUint16(sound_time_code);
    writer.WriteUint8(shot->fps);
    writer.WriteUint8(shot->wait_sound ? 1 : 0);
    writer.WriteUint8(shot->keep_palette ? 1 : 0);
    writer.WriteUint8(shot->fade_in);
    writer.WriteInt16(shot->end_layer);
    writer.WriteUint8(shot->full_screen ? 1 : 0);
    writer.WriteUint8(shot->clear_palette ? 1 : 0);
    writer.EndChunk();
    WritePalettes(writer, shot);
    
    // Arrière-plans
    if (!shot->background.empty()) {
        WriteBackgrounds(writer, shot->background);
    }
    
    // Sprites
    if (!shot->sprites.empty()) {
        WriteSprites(writer, shot->sprites);
    }
    
    // Premier-plans
    if (!shot->foreground.empty()) {
        WriteForegrounds(writer, shot->foreground);
    }
    
    // Personnages
    if (!shot->characters.empty()) {
        WriteCharacters(writer, shot->characters);
    }
    if (!shot->textes.empty()) {
        WriteTexts(writer, shot->textes);
    }
    
    for (MIDGAME_SHOT_SOUND* sound : shot->sounds) {
        if (sound->pak != nullptr && sound->pak_entry_id >= 0) {
            WriteSound(writer, sound->pak, sound->pak_entry_id, sound->frame_start);
        }
    }
    writer.EndChunk();
}

void SCAnimationArchive::WriteSound(IFFWriter& writer, PakArchive* pak, int pak_entry_id, uint16_t frame_start) {
    writer.StartChunk("SOND");
    std::string archiveName = pak->GetName();
    if (archiveName.length() < 4 || archiveName.compare(archiveName.length() - 4, 4, ".PAK") != 0) {
        archiveName += ".PAK";
    }
    AssetManager& Assets = AssetManager::getInstance();
    for (auto treRef: Assets.treEntries) {
        if (treRef.first.length() >= archiveName.length() &&
            treRef.first.compare(treRef.first.length() - archiveName.length(), archiveName.length(), archiveName) == 0) {
            archiveName = treRef.first;
            break;
        }
    }
    writer.WriteUint16(archiveName.length());
    writer.WriteString(archiveName.c_str(), archiveName.length());
    writer.WriteUint16(pak_entry_id);
    writer.WriteUint16(frame_start);
    writer.EndChunk();
}

std::string SCAnimationArchive::ArchiveName(PakArchive* pak) {
    std::string archiveName = pak->GetName();
    if (archiveName.length() < 4 || archiveName.compare(archiveName.length() - 4, 4, ".PAK") != 0) {
        archiveName += ".PAK";
    }
    AssetManager& Assets = AssetManager::getInstance();
    for (auto treRef: Assets.treEntries) {
        if (treRef.first.length() >= archiveName.length() &&
            treRef.first.compare(treRef.first.length() - archiveName.length(), archiveName.length(), archiveName) == 0) {
            archiveName = treRef.first;
            break;
        }
    }
    return archiveName;
}

void SCAnimationArchive::WritePalettes(IFFWriter& writer, const MIDGAME_SHOT* shot) {
    if (shot->palettes.empty()) {
        return;
    }
    writer.StartChunk("PALS");
    writer.WriteUint16(shot->palettes.size());
    for (const MIDGAME_PALETTE_REF& ref : shot->palettes) {
        writer.WriteUint16(ref.archive.length());
        writer.WriteString(ref.archive.c_str(), ref.archive.length());
        writer.WriteUint16(ref.entry);
    }
    writer.EndChunk();
}

void SCAnimationArchive::WriteLayerCommon(IFFWriter& writer, const MIDGAME_LAYER_COMMON& common) {
    writer.StartChunk("LAYR");
    writer.WriteInt16(common.layer_id);
    writer.WriteInt16(common.parent_id);
    writer.WriteUint16(common.frame_start);
    writer.WriteUint16(common.nb_frames);
    writer.WriteUint8(common.jitter ? 1 : 0);
    writer.WriteUint8(common.clip ? 1 : 0);
    writer.WriteInt16(common.clip_min.x);
    writer.WriteInt16(common.clip_min.y);
    writer.WriteInt16(common.clip_max.x);
    writer.WriteInt16(common.clip_max.y);
    writer.WriteInt16(common.depth);
    writer.WriteUint8(common.game_numbering ? 1 : 0);
    writer.EndChunk();
    WriteSequence(writer, common.sequence);
    if (!common.path.empty()) {
        writer.StartChunk("PATH");
        writer.WriteUint16(common.path.size());
        for (const Point2D& point : common.path) {
            writer.WriteInt16(point.x);
            writer.WriteInt16(point.y);
        }
        writer.EndChunk();
    }
}

void SCAnimationArchive::WriteSequence(IFFWriter& writer, const std::vector<uint8_t>& sequence) {
    if (sequence.empty()) {
        return;
    }
    writer.StartChunk("SEQU");
    writer.WriteUint16(sequence.size());
    for (uint8_t frame : sequence) {
        writer.WriteUint8(frame);
    }
    writer.EndChunk();
}

void SCAnimationArchive::WriteLips(IFFWriter& writer, const std::vector<MIDGAME_LIP_TEXT>& lips) {
    if (lips.empty()) {
        return;
    }
    writer.StartChunk("LIPS");
    writer.WriteUint16(lips.size());
    for (const MIDGAME_LIP_TEXT& lip : lips) {
        writer.WriteUint16(lip.frame_start);
        writer.WriteUint16(lip.text.length());
        writer.WriteString(lip.text.c_str(), lip.text.length());
    }
    writer.EndChunk();
}

// Ajouter cette nouvelle fonction pour écrire les personnages
void SCAnimationArchive::WriteCharacters(IFFWriter& writer, const std::vector<MIDGAME_SHOT_CHARACTER*>& characters) {
    writer.StartChunk("CHAR");
    for (const MIDGAME_SHOT_CHARACTER* character : characters) {
        writer.StartChunk("CHRC");
        
        // Écriture du nom du personnage
        writer.WriteUint16(character->character_name.length());
        writer.WriteString(character->character_name.c_str(), character->character_name.length());
        
        // Écriture du nom du fond
        writer.WriteUint16(character->background_name.length());
        writer.WriteString(character->background_name.c_str(), character->background_name.length());
        
        // Positions et vélocité
        writer.WriteInt16(character->position_start.x);
        writer.WriteInt16(character->position_start.y);
        writer.WriteInt16(character->position_end.x);
        writer.WriteInt16(character->position_end.y);
        writer.WriteInt16(character->velocity.x);
        writer.WriteInt16(character->velocity.y);
        
        // IDs des éléments du personnage
        writer.WriteUint8(character->palette);
        writer.WriteUint8(character->cloth_id);
        writer.WriteUint8(character->head_id);
        writer.WriteUint8(character->expression_id);
        writer.WriteUint8(character->talking ? 1 : 0);

        writer.EndChunk();
        WriteLayerCommon(writer, character->common);
        WriteLips(writer, character->lips);
    }
    writer.EndChunk();
}

void SCAnimationArchive::WriteBackgrounds(IFFWriter& writer, const std::vector<MIDGAME_SHOT_BG*>& backgrounds) {
    writer.StartChunk("BGND");
    for (const MIDGAME_SHOT_BG* bg : backgrounds) {
        WriteBackground(writer, bg);
    }
    writer.EndChunk();
}

void SCAnimationArchive::WriteBackground(IFFWriter& writer, const MIDGAME_SHOT_BG* bg) {
    writer.StartChunk("BKGD");
    
    AssetManager& Assets = AssetManager::getInstance();

    std::string archiveName = bg->face.empty() ? bg->pak->GetName() : "";
    if (!bg->face.empty()) {
        archiveName = "FACE:" + bg->face;
    } else if (archiveName.length() > 0) {
        if (archiveName.length() < 4 || archiveName.compare(archiveName.length() - 4, 4, ".PAK") != 0) {
            archiveName += ".PAK";
        }
        for (auto treRef: Assets.treEntries) {
            if (treRef.first.length() >= archiveName.length() &&
                treRef.first.compare(treRef.first.length() - archiveName.length(), archiveName.length(), archiveName) == 0) {
                archiveName = treRef.first;
                break;
            }
        }
    }

    std::string paletteArchiveName = bg->pak_palette ? bg->pak_palette->GetName() : "";
    if (paletteArchiveName.length() > 0) {    
        if (paletteArchiveName.length() < 4 || paletteArchiveName.compare(paletteArchiveName.length() - 4, 4, ".PAK") != 0) {
            paletteArchiveName += ".PAK";
        }
        for (auto treRef: Assets.treEntries) {
            if (treRef.first.length() >= paletteArchiveName.length() &&
                treRef.first.compare(treRef.first.length() - paletteArchiveName.length(), paletteArchiveName.length(), paletteArchiveName) == 0) {
                paletteArchiveName = treRef.first;
                break;
            }
        }
    }
    
    writer.WriteUint16(archiveName.length());
    writer.WriteString(archiveName.c_str(), archiveName.length());
    writer.WriteUint16(paletteArchiveName.length());
    writer.WriteString(paletteArchiveName.c_str(), paletteArchiveName.length());
    writer.WriteUint16(bg->pak_entry_id);
    writer.WriteUint16(bg->palette);
    writer.WriteUint16(bg->shapeid);
    writer.WriteInt8(bg->use_external_palette ? 1 : 0);
    // Positions et vélocité
    writer.WriteInt16(bg->position_start.x);
    writer.WriteInt16(bg->position_start.y);
    writer.WriteInt16(bg->position_end.x);
    writer.WriteInt16(bg->position_end.y);
    writer.WriteInt16(bg->velocity.x);
    writer.WriteInt16(bg->velocity.y);

    writer.EndChunk();
    WriteLayerCommon(writer, bg->common);
}

void SCAnimationArchive::WriteSprites(IFFWriter& writer, const std::vector<MIDGAME_SHOT_SPRITE*>& sprites) {
    writer.StartChunk("SPRT");
    for (const MIDGAME_SHOT_SPRITE* sprite : sprites) {
        WriteSprite(writer, sprite);
    }
    writer.EndChunk();
}

void SCAnimationArchive::WriteSprite(IFFWriter& writer, const MIDGAME_SHOT_SPRITE* sprite) {
    if (sprite->pak == nullptr && sprite->face.empty()) {
        return;
    }
    writer.StartChunk("SPRI");
    std::string archiveName = sprite->face.empty() ? ArchiveName(sprite->pak) : "FACE:" + sprite->face;
    
    std::string paletteArchiveName = sprite->pak_palette ? sprite->pak_palette->GetName() : "";
    if (paletteArchiveName.length() < 4 || paletteArchiveName.compare(paletteArchiveName.length() - 4, 4, ".PAK") != 0) {
        paletteArchiveName += ".PAK";
    }
    AssetManager& Assets = AssetManager::getInstance();
    if (paletteArchiveName.length() > 0) {    
        for (auto treRef: Assets.treEntries) {
            if (treRef.first.length() >= paletteArchiveName.length() &&
                treRef.first.compare(treRef.first.length() - paletteArchiveName.length(), paletteArchiveName.length(), paletteArchiveName) == 0) {
                paletteArchiveName = treRef.first;
                break;
            }
        }
    }
    writer.WriteUint16(archiveName.length());
    writer.WriteString(archiveName.c_str(), archiveName.length());
    
    writer.WriteUint16(paletteArchiveName.length());
    writer.WriteString(paletteArchiveName.c_str(), paletteArchiveName.length());

    writer.WriteUint16(sprite->pak_entry_id);
    writer.WriteUint16(sprite->shapeid);
    writer.WriteUint16(sprite->palette);
    writer.WriteUint8(sprite->use_external_palette ? 1 : 0);

    writer.WriteInt16(sprite->position_start.x);
    writer.WriteInt16(sprite->position_start.y);
    writer.WriteInt16(sprite->position_end.x);
    writer.WriteInt16(sprite->position_end.y);
    writer.WriteInt16(sprite->velocity.x);
    writer.WriteInt16(sprite->velocity.y);
    writer.WriteUint8(sprite->keep_first_frame);
    writer.WriteUint8(sprite->repeat_animation);
    writer.WriteInt16(sprite->start_frame);
    writer.WriteInt16(sprite->end_frame);
    writer.EndChunk();
    WriteLayerCommon(writer, sprite->common);
}

void SCAnimationArchive::WriteTexts(IFFWriter &writer, const std::vector<MIDGAME_TEXT_LINE *>& textes) {
    writer.StartChunk("TEXT");
    for (MIDGAME_TEXT_LINE *text : textes) {
        writer.StartChunk("TEXE");
        writer.WriteUint16(text->key.length());
        writer.WriteString(text->key.c_str(), text->key.length());
        writer.WriteUint8(text->id);
        writer.WriteUint8(text->font_id);
        writer.WriteUint16(text->nb_frames);
        writer.WriteInt16(text->position_start.x);
        writer.WriteInt16(text->position_start.y);
        writer.WriteInt16(text->position_end.x);
        writer.WriteInt16(text->position_end.y);
        writer.WriteUint8(text->color);
        writer.WriteUint16(text->frame_start);
        writer.WriteUint8(text->align);
        writer.EndChunk();
    }
    writer.EndChunk();
}

void SCAnimationArchive::WriteForegrounds(IFFWriter& writer, const std::vector<MIDGAME_SHOT_BG*>& foregrounds) {
    writer.StartChunk("FGND");
    for (const MIDGAME_SHOT_BG* fg : foregrounds) {
        writer.StartChunk("FRGD");
        
        std::string archiveName = fg->face.empty() ? fg->pak->GetName() : "FACE:" + fg->face;
        if (fg->face.empty() && (archiveName.length() < 4 || archiveName.compare(archiveName.length() - 4, 4, ".PAK") != 0)) {
            archiveName += ".PAK";
        }
        
        std::string paletteArchiveName = fg->pak_palette ? fg->pak_palette->GetName() : "";
        if (paletteArchiveName.length() < 4 || paletteArchiveName.compare(paletteArchiveName.length() - 4, 4, ".PAK") != 0) {
            paletteArchiveName += ".PAK";
        }
        AssetManager& Assets = AssetManager::getInstance();
        for (auto treRef: Assets.treEntries) {
            if (!fg->face.empty()) {
                break;
            }
            if (treRef.first.length() >= archiveName.length() &&
                treRef.first.compare(treRef.first.length() - archiveName.length(), archiveName.length(), archiveName) == 0) {
                archiveName = treRef.first;
                break;
            }
        }
        if (paletteArchiveName.length() > 0) {    
            for (auto treRef: Assets.treEntries) {
                if (treRef.first.length() >= paletteArchiveName.length() &&
                    treRef.first.compare(treRef.first.length() - paletteArchiveName.length(), paletteArchiveName.length(), paletteArchiveName) == 0) {
                    paletteArchiveName = treRef.first;
                    break;
                }
            }
        }
        writer.WriteUint16(archiveName.length());
        writer.WriteString(archiveName.c_str(), archiveName.length());
        writer.WriteUint16(paletteArchiveName.length());
        writer.WriteString(paletteArchiveName.c_str(), paletteArchiveName.length());
        writer.WriteUint16(fg->pak_entry_id);
        writer.WriteUint16(fg->palette);
        writer.WriteUint16(fg->shapeid);
        writer.WriteUint8(fg->use_external_palette ? 1 : 0);
        
        // Positions et vélocité
        writer.WriteInt16(fg->position_start.x);
        writer.WriteInt16(fg->position_start.y);
        writer.WriteInt16(fg->position_end.x);
        writer.WriteInt16(fg->position_end.y);
        writer.WriteInt16(fg->velocity.x);
        writer.WriteInt16(fg->velocity.y);

        writer.EndChunk();
        WriteLayerCommon(writer, fg->common);
    }
    writer.EndChunk();
}

void SCAnimationArchive::HandleTEXT(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    
    std::unordered_map<std::string, std::function<void(uint8_t*, size_t)>> events;
    events["TEXE"] = std::bind(&SCAnimationArchive::HandleTEXT_TEXE, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, events);
}
void SCAnimationArchive::HandleTEXT_TEXE(uint8_t* data, size_t size) {
    if (currentShot) {
        ByteStream stream(data, size);
        
        MIDGAME_TEXT_LINE *text;
        text = new MIDGAME_TEXT_LINE();
        uint16_t keyLength = stream.ReadUShort();
        text->key = stream.ReadString(keyLength);
        text->id = stream.ReadByte();
        text->font_id = stream.ReadByte();
        text->nb_frames = stream.ReadUShort();
        text->position_start.x = stream.ReadShort();
        text->position_start.y = stream.ReadShort();
        text->position_end.x = stream.ReadShort();
        text->position_end.y = stream.ReadShort();
        text->color = stream.ReadByte();
        if (!stream.IsEndOfStream()) {
            text->frame_start = stream.ReadUShort();
        }
        if (!stream.IsEndOfStream()) {
            text->align = stream.ReadByte();
        }
        currentShot->textes.push_back(text);
    }
}
void SCAnimationArchive::HandleANIM(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    
    std::unordered_map<std::string, std::function<void(uint8_t*, size_t)>> events;
    events["HEAD"] = std::bind(&SCAnimationArchive::HandleHEAD, this, std::placeholders::_1, std::placeholders::_2);
    events["SHOT"] = std::bind(&SCAnimationArchive::HandleSHOT, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, events);
}

void SCAnimationArchive::HandleHEAD(uint8_t* data, size_t size) {
    ByteStream stream(data, size);
    stream.ReadUInt32LE();
    if (!stream.IsEndOfStream()) {
        this->version = stream.ReadUInt32LE();
    }
}

void SCAnimationArchive::HandleLAYR(uint8_t* data, size_t size) {
    if (currentCommon) {
        ByteStream stream(data, size);
        currentCommon->layer_id = stream.ReadShort();
        currentCommon->parent_id = stream.ReadShort();
        currentCommon->frame_start = stream.ReadUShort();
        currentCommon->nb_frames = stream.ReadUShort();
        currentCommon->jitter = stream.ReadByte() != 0;
        currentCommon->clip = stream.ReadByte() != 0;
        currentCommon->clip_min.x = stream.ReadShort();
        currentCommon->clip_min.y = stream.ReadShort();
        currentCommon->clip_max.x = stream.ReadShort();
        currentCommon->clip_max.y = stream.ReadShort();
        if (!stream.IsEndOfStream()) {
            currentCommon->depth = stream.ReadShort();
            currentCommon->game_numbering = stream.ReadByte() != 0;
        }
    }
}

void SCAnimationArchive::HandlePATH(uint8_t* data, size_t size) {
    if (currentCommon) {
        ByteStream stream(data, size);
        uint16_t count = stream.ReadUShort();
        currentCommon->path.clear();
        for (uint16_t i = 0; i < count; i++) {
            Point2D point;
            point.x = stream.ReadShort();
            point.y = stream.ReadShort();
            currentCommon->path.push_back(point);
        }
    }
}

void SCAnimationArchive::HandleSEQU(uint8_t* data, size_t size) {
    if (currentCommon) {
        ByteStream stream(data, size);
        uint16_t count = stream.ReadUShort();
        currentCommon->sequence.clear();
        for (uint16_t i = 0; i < count; i++) {
            currentCommon->sequence.push_back(stream.ReadByte());
        }
    }
}

void SCAnimationArchive::HandleLIPS(uint8_t* data, size_t size) {
    if (currentCharacter) {
        ByteStream stream(data, size);
        uint16_t count = stream.ReadUShort();
        currentCharacter->lips.clear();
        for (uint16_t i = 0; i < count; i++) {
            MIDGAME_LIP_TEXT lip;
            lip.frame_start = stream.ReadUShort();
            uint16_t length = stream.ReadUShort();
            if (length > 0) {
                lip.text = stream.ReadString(length);
            }
            currentCharacter->lips.push_back(lip);
        }
    }
}

// Mettre à jour HandleSHOT pour inclure les gestionnaires de personnages
void SCAnimationArchive::HandleSHOT(uint8_t* data, size_t size) {
    // Créer un nouveau shot
    currentShot = new MIDGAME_SHOT();
    currentShot->version = this->version;

    IFFSaxLexer lexer;
    
    std::unordered_map<std::string, std::function<void(uint8_t*, size_t)>> events;
    events["SHTP"] = std::bind(&SCAnimationArchive::HandleSHTP, this, std::placeholders::_1, std::placeholders::_2);
    events["BGND"] = std::bind(&SCAnimationArchive::HandleBGND, this, std::placeholders::_1, std::placeholders::_2);
    events["SPRT"] = std::bind(&SCAnimationArchive::HandleSPRT, this, std::placeholders::_1, std::placeholders::_2);
    events["FGND"] = std::bind(&SCAnimationArchive::HandleFGND, this, std::placeholders::_1, std::placeholders::_2);
    events["SOND"] = std::bind(&SCAnimationArchive::HandleSOND, this, std::placeholders::_1, std::placeholders::_2);
    events["CHAR"] = std::bind(&SCAnimationArchive::HandleCHAR, this, std::placeholders::_1, std::placeholders::_2);
    events["TEXT"] = std::bind(&SCAnimationArchive::HandleTEXT, this, std::placeholders::_1, std::placeholders::_2);
    events["PALS"] = std::bind(&SCAnimationArchive::HandlePALS, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, events);
    
    // Ajouter le shot à la liste
    shots->push_back(currentShot);
    currentShot = nullptr;
}

// Ajouter ces nouvelles fonctions pour lire les personnages
void SCAnimationArchive::HandleCHAR(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    
    std::unordered_map<std::string, std::function<void(uint8_t*, size_t)>> events;
    events["CHRC"] = std::bind(&SCAnimationArchive::HandleCHRC, this, std::placeholders::_1, std::placeholders::_2);
    events["LAYR"] = std::bind(&SCAnimationArchive::HandleLAYR, this, std::placeholders::_1, std::placeholders::_2);
    events["PATH"] = std::bind(&SCAnimationArchive::HandlePATH, this, std::placeholders::_1, std::placeholders::_2);
    events["LIPS"] = std::bind(&SCAnimationArchive::HandleLIPS, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, events);
}

void SCAnimationArchive::HandleCHRC(uint8_t* data, size_t size) {
    if (currentShot) {
        ByteStream stream(data, size);
        
        MIDGAME_SHOT_CHARACTER* character = new MIDGAME_SHOT_CHARACTER();
        
        // Lecture du nom du personnage
        uint16_t charNameLength = stream.ReadUShort();
        character->character_name = stream.ReadString(charNameLength);
        ConvAssetManager& ConvAssets = ConvAssetManager::getInstance();
        character->image = ConvAssets.GetCharFace(character->character_name)->appearances;
        // Lecture du nom du fond
        uint16_t bgNameLength = stream.ReadUShort();
        if (bgNameLength > 0)
            character->background_name = stream.ReadString(bgNameLength);
        
        // Lecture des positions et vélocité
        character->position_start.x = stream.ReadShort();
        character->position_start.y = stream.ReadShort();
        character->position_end.x = stream.ReadShort();
        character->position_end.y = stream.ReadShort();
        character->velocity.x = stream.ReadShort();
        character->velocity.y = stream.ReadShort();
        
        // Lecture des IDs des éléments du personnage
        character->palette = stream.ReadByte();
        // version 1 : 0 voulait dire « pas de palette » (sauf pour les fonds)
        if (this->version < 2 && character->palette == 0) {
            character->palette = 255;
        }
        character->cloth_id = stream.ReadByte();
        character->head_id = stream.ReadByte();
        character->expression_id = stream.ReadByte();
        character->talking = stream.ReadByte() != 0;
        
        // Chargement des images du personnage
        // Cette partie dépend de comment vous stockez/chargez les images des personnages
        // et peut nécessiter une logique spécifique selon votre système de ressources

        currentShot->characters.push_back(character);
        currentCharacter = character;
        currentCommon = &character->common;
    }
}

void SCAnimationArchive::HandleSHTP(uint8_t* data, size_t size) {
    if (currentShot) {
        ByteStream stream(data, size);
        currentShot->nbframe = stream.ReadInt32LE();
        currentShot->music = stream.ReadByte();
        currentShot->sound_time_code = stream.ReadUShort();
        if (!stream.IsEndOfStream()) {
            currentShot->fps = stream.ReadByte();
            currentShot->wait_sound = stream.ReadByte() != 0;
            currentShot->keep_palette = stream.ReadByte() != 0;
        }
        if (!stream.IsEndOfStream()) {
            currentShot->fade_in = stream.ReadByte();
            currentShot->end_layer = stream.ReadShort();
            currentShot->full_screen = stream.ReadByte() != 0;
            currentShot->clear_palette = stream.ReadByte() != 0;
        }
    }
}

void SCAnimationArchive::HandlePALS(uint8_t* data, size_t size) {
    if (currentShot) {
        ByteStream stream(data, size);
        uint16_t count = stream.ReadUShort();
        for (uint16_t i = 0; i < count; i++) {
            MIDGAME_PALETTE_REF ref;
            uint16_t length = stream.ReadUShort();
            if (length > 0) {
                ref.archive = stream.ReadString(length);
            }
            ref.entry = stream.ReadUShort();
            if (ref.archive.rfind("FACEPAL:", 0) != 0) {
                ref.pak = FindOrLoadPakArchive(ref.archive);
            }
            currentShot->palettes.push_back(ref);
        }
    }
}

RSImageSet* SCAnimationArchive::LoadLayerImage(const std::string& archiveName, PakArchive* pak, int entryId,
                                               std::string& face, MIDGAME_LAYER_COMMON& common) {
    if (archiveName.rfind("FACE:", 0) == 0) {
        face = archiveName.substr(5);
        common.image_offset = 1;
        return ConvAssetManager::getInstance().GetCharFace(face)->appearances;
    }
    if (pak == nullptr) {
        return nullptr;
    }
    PakEntry* entry = pak->GetEntry(entryId);
    if (entry == nullptr) {
        return nullptr;
    }
    RSImageSet* tmp_img = new RSImageSet();
    PakArchive* pk = new PakArchive();
    pk->InitFromRAM("temp", entry->data, entry->size);
    tmp_img->InitFromPakArchive(pk, 0);
    common.image_offset = 0;
    if (tmp_img->GetNumImages() == 0) {
        delete tmp_img;
        tmp_img = new RSImageSet();
        tmp_img->InitFromPakEntry(entry);
        common.image_offset = 1;
    }
    return tmp_img;
}

void SCAnimationArchive::HandleBGND(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    std::unordered_map<std::string, std::function<void(uint8_t*, size_t)>> events;
    events["BKGD"] = std::bind(&SCAnimationArchive::HandleBKGD, this, std::placeholders::_1, std::placeholders::_2);
    events["LAYR"] = std::bind(&SCAnimationArchive::HandleLAYR, this, std::placeholders::_1, std::placeholders::_2);
    events["PATH"] = std::bind(&SCAnimationArchive::HandlePATH, this, std::placeholders::_1, std::placeholders::_2);
    events["SEQU"] = std::bind(&SCAnimationArchive::HandleSEQU, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, events);
}

void SCAnimationArchive::HandleBKGD(uint8_t* data, size_t size) {
    if (currentShot) {
        ByteStream stream(data, size);
        
        MIDGAME_SHOT_BG* bg = new MIDGAME_SHOT_BG();
        
        // Lire le nom de l'archive
        uint16_t nameLength = stream.ReadUShort();
        std::string archiveName = stream.ReadString(nameLength);
        bg->pak = FindOrLoadPakArchive(archiveName);
        
        nameLength = stream.ReadUShort();
        if (nameLength > 0) {
            std::string paletteArchiveName = stream.ReadString(nameLength);
            // Trouver ou charger l'archive
            
            if (!paletteArchiveName.empty()) {
                bg->pak_palette = FindOrLoadPakArchive(paletteArchiveName);
            } else {
                bg->pak_palette = nullptr;
            }
        } else {
            bg->pak_palette = nullptr;
        }
        
        bg->pak_entry_id = stream.ReadUShort();
        bg->palette = stream.ReadUShort();
        bg->shapeid = stream.ReadUShort();
        bg->use_external_palette = stream.ReadByte() != 0;
        // Lire les positions et la vélocité
        bg->position_start.x = stream.ReadShort();
        bg->position_start.y = stream.ReadShort();
        bg->current_position = {bg->position_start.x, bg->position_start.y};
        bg->position_end.x = stream.ReadShort();
        bg->position_end.y = stream.ReadShort();
        bg->velocity.x = stream.ReadShort();
        bg->velocity.y = stream.ReadShort();
        
        bg->image = LoadLayerImage(archiveName, bg->pak, bg->pak_entry_id, bg->face, bg->common);
        if (bg->image != nullptr && bg->face.empty() && bg->image->palettes.size() > 0) {
            bg->pal = bg->image->palettes[0];
        }
        currentShot->background.push_back(bg);
        currentCommon = &bg->common;
    }
}

void SCAnimationArchive::HandleSPRT(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    
    std::unordered_map<std::string, std::function<void(uint8_t*, size_t)>> events;
    events["SPRI"] = std::bind(&SCAnimationArchive::HandleSPRI, this, std::placeholders::_1, std::placeholders::_2);
    events["LAYR"] = std::bind(&SCAnimationArchive::HandleLAYR, this, std::placeholders::_1, std::placeholders::_2);
    events["PATH"] = std::bind(&SCAnimationArchive::HandlePATH, this, std::placeholders::_1, std::placeholders::_2);
    events["SEQU"] = std::bind(&SCAnimationArchive::HandleSEQU, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, events);
}

void SCAnimationArchive::HandleSPRI(uint8_t* data, size_t size) {
    if (currentShot) {
        ByteStream stream(data, size);
        
        MIDGAME_SHOT_SPRITE* sprite = new MIDGAME_SHOT_SPRITE();
        
        // Lire le nom de l'archive
        uint16_t nameLength = stream.ReadUShort();
        std::string archiveName = stream.ReadString(nameLength);
        
        sprite->pak = FindOrLoadPakArchive(archiveName);
        
        uint16_t palNameLength = stream.ReadUShort();
        if (palNameLength > 0) {
            std::string paletteArchiveName = stream.ReadString(palNameLength);
            if (!paletteArchiveName.empty()) {
                sprite->pak_palette = FindOrLoadPakArchive(paletteArchiveName);
            } else {
                sprite->pak_palette = nullptr;
            }
        } else {
            sprite->pak_palette = nullptr;
        }
        
        sprite->pak_entry_id = stream.ReadUShort(); 
        sprite->shapeid = stream.ReadUShort();
        sprite->palette = stream.ReadUShort();
        if (this->version < 2 && sprite->palette == 0) {
            sprite->palette = 255;
        }
        sprite->use_external_palette = stream.ReadByte() != 0;
        
        sprite->position_start.x = stream.ReadShort();
        sprite->position_start.y = stream.ReadShort();
        sprite->position_end.x = stream.ReadShort();
        sprite->position_end.y = stream.ReadShort();
        sprite->velocity.x = stream.ReadShort();
        sprite->velocity.y = stream.ReadShort();
        sprite->keep_first_frame = stream.ReadByte();
        sprite->repeat_animation = stream.ReadByte();
        sprite->start_frame = stream.ReadShort();
        sprite->end_frame = stream.ReadShort();
        sprite->image = LoadLayerImage(archiveName, sprite->pak, sprite->pak_entry_id, sprite->face, sprite->common);
        if (sprite->image != nullptr && sprite->face.empty() && sprite->image->palettes.size() > 0) {
            sprite->pal = sprite->image->palettes[0];
        }
        currentShot->sprites.push_back(sprite);
        currentSprite = sprite;
        currentCommon = &sprite->common;
    }
}

void SCAnimationArchive::HandleFGND(uint8_t* data, size_t size) {
    IFFSaxLexer lexer;
    
    std::unordered_map<std::string, std::function<void(uint8_t*, size_t)>> events;
    events["FRGD"] = std::bind(&SCAnimationArchive::HandleFRGD, this, std::placeholders::_1, std::placeholders::_2);
    events["LAYR"] = std::bind(&SCAnimationArchive::HandleLAYR, this, std::placeholders::_1, std::placeholders::_2);
    events["PATH"] = std::bind(&SCAnimationArchive::HandlePATH, this, std::placeholders::_1, std::placeholders::_2);
    events["SEQU"] = std::bind(&SCAnimationArchive::HandleSEQU, this, std::placeholders::_1, std::placeholders::_2);

    
    lexer.InitFromRAM(data, size, events);
}

void SCAnimationArchive::HandleFRGD(uint8_t* data, size_t size) {
    if (currentShot) {
        ByteStream stream(data, size);
        
        MIDGAME_SHOT_BG* fg = new MIDGAME_SHOT_BG();
        
        // Lire le nom de l'archive
        uint16_t nameLength = stream.ReadUShort();
        std::string archiveName = stream.ReadString(nameLength);
        fg->pak = FindOrLoadPakArchive(archiveName);
        // Lire le nom de l'archive de palette
        uint16_t palNameLength = stream.ReadUShort();
        if (palNameLength > 0) {
            std::string paletteArchiveName = stream.ReadString(palNameLength);
            if (!paletteArchiveName.empty()) {
                fg->pak_palette = FindOrLoadPakArchive(paletteArchiveName);
            } else {
                fg->pak_palette = nullptr;
            }
        } else {
            fg->pak_palette = nullptr;
        }
        
        fg->pak_entry_id = stream.ReadUShort();
        fg->palette = stream.ReadUShort();
        if (this->version < 2 && fg->palette == 0) {
            fg->palette = 255;
        }
        fg->shapeid = stream.ReadUShort();
        fg->use_external_palette = stream.ReadByte() != 0;
        
        // Lire les positions et la vélocité
        fg->position_start.x = stream.ReadShort();
        fg->position_start.y = stream.ReadShort();
        fg->current_position = {fg->position_start.x, fg->position_start.y};
        fg->position_end.x = stream.ReadShort();
        fg->position_end.y = stream.ReadShort();
        fg->velocity.x = stream.ReadShort();
        fg->velocity.y = stream.ReadShort();
        
        fg->image = LoadLayerImage(archiveName, fg->pak, fg->pak_entry_id, fg->face, fg->common);
        if (fg->image != nullptr && fg->face.empty() && fg->image->palettes.size() > 0) {
            fg->pal = fg->image->palettes[0];
        }
        
        currentShot->foreground.push_back(fg);
        currentCommon = &fg->common;
    }
}

void SCAnimationArchive::HandleSOND(uint8_t* data, size_t size) {
    if (currentShot) {
        ByteStream stream(data, size);
        
        MIDGAME_SHOT_SOUND* sound = new MIDGAME_SHOT_SOUND();
        uint16_t nameLength = stream.ReadUShort();
        std::string archiveName = stream.ReadString(nameLength);
        sound->pak = FindOrLoadPakArchive(archiveName);
        sound->pak_entry_id = stream.ReadUShort();
        if (currentShot->sound_time_code > 0) {
            sound->frame_start = currentShot->sound_time_code - 1;
        }
        if (!stream.IsEndOfStream()) {
            sound->frame_start = stream.ReadUShort();
        }

        if (sound->pak != nullptr) {
            PakEntry* soundEntry = sound->pak->GetEntry(sound->pak_entry_id);
            if (soundEntry != nullptr) {
                sound->sound = new MIDGAME_SOUND();
                sound->sound->data = soundEntry->data;
                sound->sound->size = soundEntry->size;
            }
        }
        currentShot->sounds.push_back(sound);
    }
}

PakArchive* SCAnimationArchive::FindOrLoadPakArchive(const std::string& name) {
    AssetManager& Assets = AssetManager::getInstance();
    // Sinon, tenter de charger l'archive depuis les assets
    TreEntry* entry = Assets.GetEntryByName(name.c_str());
    if (entry != nullptr) {
        PakArchive* archive = new PakArchive();
        archive->InitFromRAM(name.c_str(), entry->data, entry->size);
        
        // Ajouter à la liste des archives chargées
        PakArchiveRef ref;
        ref.name = name;
        ref.archive = archive;
        pakArchives.push_back(ref);
        
        return archive;
    }
    
    return nullptr;
}