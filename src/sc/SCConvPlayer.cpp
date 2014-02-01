//
//  SCConvPlayer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


SCConvPlayer::SCConvPlayer():
    conversationID(0)
{
    
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

void SCConvPlayer::ReadNextFrame(void){
    
    if (read == size){
        Stop();
        return;
    }
    
    uint8_t* startPos = conv.GetPosition();
    
    uint8_t type = conv.ReadByte();
    
    switch (type) {
        case GROUP_SHOT:  // Group plan
        {
            char* location = (char*)conv.GetPosition();
            printf("WIDEPLAN : LOCATION: '%s'\n",location);
            conv.MoveForward(8+1);
            break;
        }
        case CLOSEUP:  // Person talking
        {
            uint8_t* speakerName = conv.GetPosition();
            uint8_t* set         = conv.GetPosition() + 0xA;
            uint8_t* sentence         = conv.GetPosition() + 0x17;
            
            
            conv.MoveForward(0x17 + strlen((char*)sentence)+1);
            uint8_t color = conv.ReadByte(); // Color ?
            
            printf("CLOSEUP: WHO: '%8s' WHERE: '%8s'     WHAT: '%s' (%2X)\n",speakerName,set,sentence,color);
            break;
        }
        case CLOSEUP_CONTINUATION:  // Same person keep talking
        {
            uint8_t* sentence         = conv.GetPosition();
            conv.MoveForward(strlen((char*)sentence)+1);
            printf("MORETEX:                                       WHAT: '%s'\n",sentence);
            break;
        }
        case YESNOCHOICE_BRANCH1:  // Choice Offsets are question
        {
            printf("CHOICE YES/NO : %X.\n",type);
            //Looks like first byte is the offset to skip if the answer is no.
            /*uint8_t noOffset  =*/  conv.ReadByte();
            /*uint8_t yesOffset  =*/ conv.ReadByte();
            break;
        }
        case YESNOCHOICE_BRANCH2:  // Choice offset after first branch
        {
            printf("CHOICE YES/NO : %X.\n",type);
            //Looks like first byte is the offset to skip if the answer is no.
            /*uint8_t yesOffset  =*/ conv.ReadByte();
            /*uint8_t noOffset  =*/  conv.ReadByte();
            break;
        }
        case GROUP_SHOT_ADD_CHARCTER:  // Add person to GROUP
        {
            printf("WIDEPLAN ADD PARTICIPANT: '%s'\n",conv.GetPosition());
            conv.MoveForward(0xD);
            break;
        }
        case GROUP_SHOT_CHARCTR_TALK:  // Make group character talk
        {
            char* who = (char*)conv.GetPosition();
            conv.MoveForward(0xE);
            char* sentence = (char*)conv.GetPosition();
            conv.MoveForward(strlen(sentence)+1);
            printf("WIDEPLAN PARTICIPANT TALKING: who: '%s' WHAT '%s'\n",who,sentence);
            
            
            break;
        }
        case SHOW_TEXT:  // Show text
        {
            /*uint8_t color= */  conv.ReadByte();
            char* sentence = (char*)conv.GetPosition();
            printf("Show Text: '%s' \n",sentence);
            conv.MoveForward(strlen(sentence)+1);
            
            break;
        }
        case 0xE:
        {
            uint8_t unkn  = conv.ReadByte();
            uint8_t unkn1  = conv.ReadByte();
            printf("Unknown usage Flag 0xE: (0x%2X 0x%2X) \n",unkn,unkn1);
            break;
        }
        case CHOOSE_WINGMAN:  // Wingman selection trigger
        {
            printf("Open pilot selection screen with current BG.\n");
            break;
        }
        default:
            printf("Unknown opcode: %X.\n",type);
            Stop();
            return ;
            break;
    }
    
    
    read += conv.GetPosition() - startPos;
}

void SCConvPlayer::SetArchive(PakEntry* convPakEntry){
    
    if (convPakEntry->size == 0){
        Game.Log("Conversation entry is empty: Unable to load it.\n");
        Stop();
        return;
    }
    
    this->size = convPakEntry->size;
    
    this->conv.Set(convPakEntry->data);
    
}


void SCConvPlayer::SetID(int32_t id){
    
    TreEntry* convEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONV.PAK");
    
    PakArchive convPak;
    convPak.InitFromRAM("CONV.PAK", convEntry->data, convEntry->size);
    //convPak.List(stdout);
    
    if (convPak.GetNumEntries() <= id){
        Stop();
        Game.Log("Cannot load conversation id (max convID is %lu).",convPak.GetNumEntries()-1);
        return;
    }
    
    SetArchive(convPak.GetEntry(id));
    
}

void SCConvPlayer::Init( ){
    
    TreEntry* convPalettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVPALS.PAK");
    PakArchive convPalettePak;
    convPalettePak.InitFromRAM("CONVPALS.PAK", convPalettesEntry->data, convPalettesEntry->size);
    
    
    //Build location database
    
    //Build character database
}

bool SCConvPlayer::IsFrameExpired(void){
    
    //A frame expires either after a player press a key, click or 6 seconds elapse.
    
    
    return false;
}

void SCConvPlayer::RunFrame(void){
    
    Game.Log("Conversation %d over.\n",this->conversationID);
    Stop();
    
    //If frame needs to be update
    bool expired = IsFrameExpired();
    if ( expired )
        ReadNextFrame();
    
    
}
