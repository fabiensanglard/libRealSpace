#include "precomp.h"
#include <string>

void RSSound::init(AssetManager *assetManager) {
    TreEntry *tre = assetManager->GetEntryByName(Assets.intel_root_path + "SPEECH.PAK");
    if (tre != nullptr) {
        PakArchive *pak = new PakArchive();
        pak->InitFromRAM("SPEECH.PAK", tre->data, tre->size);
        if (pak->IsReady()) {
            for (size_t i = 0; i < pak->GetNumEntries(); ++i) {
                PakEntry *entry = pak->GetEntry(i);
                
                if (entry != nullptr) {
                    InGameVoice *voice = new InGameVoice();
                    voice->id = i;
                    PakArchive *subPak = new PakArchive();
                    std::string subPakName = "voice"+std::to_string(i);
                    subPak->InitFromRAM(subPakName.c_str(), entry->data, entry->size);
                    for (size_t j = 0; j < subPak->GetNumEntries(); ++j) {
                        PakEntry *subEntry = subPak->GetEntry(j);
                        if (subEntry != nullptr) {
                            MemSound *mem_sound = new MemSound();
                            mem_sound->data = subEntry->data;
                            mem_sound->size = subEntry->size;
                            mem_sound->id = j; // Assign an ID based on the entry index
                            voice->messages[mem_sound->id] = mem_sound;
                        }
                    }
                    delete subPak;
                    inGameVoices[voice->id] = voice;                   
                }
            }
        }
    }
    
    tre = assetManager->GetEntryByName(Assets.sound_root_path + "DSOUNDFX.PAK");
    if (tre != nullptr) {
        PakArchive *pak = new PakArchive();
        pak->InitFromRAM("DSOUNDFX.PAK", tre->data, tre->size);
        if (pak->IsReady()) {
            for (size_t i = 0; i < pak->GetNumEntries(); ++i) {
                PakEntry *entry = pak->GetEntry(i);
                PakArchive *subPak = new PakArchive();
                std::string subPakName = "soundfx"+std::to_string(i);
                subPak->InitFromRAM(subPakName.c_str(), entry->data, entry->size);
                if (subPak->IsReady()) {
                    for (size_t j = 0; j < subPak->GetNumEntries(); ++j) {
                        PakEntry *subEntry = subPak->GetEntry(j);
                        if (subEntry != nullptr) {
                            MemSound *mem_sound = new MemSound();
                            mem_sound->data = subEntry->data;
                            mem_sound->size = subEntry->size;
                            mem_sound->id = j; // Assign an ID based on the entry index
                            sounds.push_back(mem_sound);
                        }
                    }
                }
            }
        }
    }
}