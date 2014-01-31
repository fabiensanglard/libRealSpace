//
//  SCConvPlayer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//


#include "precomp.h"


SCConvPlayer::SCConvPlayer(){
    
}

SCConvPlayer::~SCConvPlayer(){
    
}

#define GROUP_SHOT              0x00
#define GROUP_SHOT_ADD_CHARCTER 0x01
#define GROUP_SHOT_CHARCTR_TALK 0x02
#define CLOSEUP                 0x03
#define CLOSEUP_CONTINUATION    0x04
#define SHOW_TEXT               0x0A
#define YESNOCHOICE_BRANCH1     0x0B
#define YESNOCHOICE_BRANCH2     0x0C
#define UNKNOWN                 0x0E
#define CHOOSE_WINGMAN          0x0F

size_t SCConvPlayer::ReadConvFrame(ByteStream* reader){
    
    uint8_t* startPos = reader->GetPosition();
    
    uint8_t type = reader->ReadByte();
    
    switch (type) {
        case GROUP_SHOT:  // Group plan
        {
            char* location = (char*)reader->GetPosition();
            printf("WIDEPLAN : LOCATION: '%s'\n",location);
            reader->MoveForward(8+1);
            break;
        }
        case CLOSEUP:  // Person talking
        {
            uint8_t* speakerName = reader->GetPosition();
            uint8_t* set         = reader->GetPosition() + 0xA;
            uint8_t* sentence         = reader->GetPosition() + 0x17;
            
            
            reader->MoveForward(0x17 + strlen((char*)sentence)+1);
            uint8_t color = reader->ReadByte(); // Color ?
            
            printf("CLOSEUP: WHO: '%8s' WHERE: '%8s'     WHAT: '%s' (%2X)\n",speakerName,set,sentence,color);
            break;
        }
        case CLOSEUP_CONTINUATION:  // Same person keep talking
        {
            uint8_t* sentence         = reader->GetPosition();
            reader->MoveForward(strlen((char*)sentence)+1);
            printf("MORETEX:                                       WHAT: '%s'\n",sentence);
            break;
        }
        case YESNOCHOICE_BRANCH1:  // Choice Offsets are question
        {
            printf("CHOICE YES/NO : %X.\n",type);
            //Looks like first byte is the offset to skip if the answer is no.
            /*uint8_t noOffset  =*/ reader->ReadByte();
            /*uint8_t yesOffset  =*/ reader->ReadByte();
            break;
        }
        case YESNOCHOICE_BRANCH2:  // Choice offset after first branch
        {
            printf("CHOICE YES/NO : %X.\n",type);
            //Looks like first byte is the offset to skip if the answer is no.
            /*uint8_t yesOffset  =*/ reader->ReadByte();
            /*uint8_t noOffset  =*/ reader->ReadByte();
            break;
        }
        case GROUP_SHOT_ADD_CHARCTER:  // Add person to GROUP
        {
            printf("WIDEPLAN ADD PARTICIPANT: '%s'\n",reader->GetPosition());
            reader->MoveForward(0xD);
            break;
        }
        case GROUP_SHOT_CHARCTR_TALK:  // Make group character talk
        {
            char* who = (char*)reader->GetPosition();
            reader->MoveForward(0xE);
            char* sentence = (char*)reader->GetPosition();
            reader->MoveForward(strlen(sentence)+1);
            printf("WIDEPLAN PARTICIPANT TALKING: who: '%s' WHAT '%s'\n",who,sentence);
            
            
            break;
        }
        case SHOW_TEXT:  // Show text
        {
            /*uint8_t color= */  reader->ReadByte();
            char* sentence = (char*)reader->GetPosition();
            printf("Show Text: '%s' \n",sentence);
            reader->MoveForward(strlen(sentence)+1);
            
            break;
        }
        case 0xE:
        {
            uint8_t unkn  = reader->ReadByte();
            uint8_t unkn1  = reader->ReadByte();
            printf("Unknown usage Flag 0xE: (0x%2X 0x%2X) \n",unkn,unkn1);
            break;
        }
        case CHOOSE_WINGMAN:  // Wingman selection trigger
        {
            printf("Open pilot selection screen with currennt BG.\n");
            break;
        }
        default:
            printf("Unknown opcode: %X.\n",type);
            return 9999;
            break;
    }
    
    
    return reader->GetPosition() - startPos;
}

void SCConvPlayer::ParseConvArchive(PakEntry* conv){
    
    if (conv->size == 0){
        Game.Log("Conversation entry is empty: Unable to load it.\n");
        Stop();
        return;
    }
    
    ByteStream planReader ;
    planReader.Set(conv->data);
    
    size_t byteToRead = conv->size;
    
    while(byteToRead > 0){
        size_t byteRead = ReadConvFrame(&planReader);
        byteToRead -= byteRead;
    }
    
}


void SCConvPlayer::SetConvID(int32_t id){
    
    TreEntry* convEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONV.PAK");
    
    PakArchive convPak;
    convPak.InitFromRAM("CONV.PAK", convEntry->data, convEntry->size);
    //convPak.List(stdout);
    
    if (convPak.GetNumEntries() <= id){
        Stop();
        Game.Log("Cannot load conversation id (max convID is %lu).",convPak.GetNumEntries()-1);
        return;
    }
    
    
    
    
    ParseConvArchive(convPak.GetEntry(id));
    
}

void SCConvPlayer::Init( ){
    
    TreEntry* convPalettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVPALS.PAK");
    PakArchive convPalettePak;
    convPalettePak.InitFromRAM("CONVPALS.PAK", convPalettesEntry->data, convPalettesEntry->size);
    
}

void SCConvPlayer::RunFrame(void){
    Stop();
}
