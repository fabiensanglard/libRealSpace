//
//  SCConvPlayer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


SCConvPlayer::SCConvPlayer():
    conversationID(0),
    initialized(false)
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

void SCConvPlayer::Focus(void) {
    IActivity::Focus();
    Screen.SetTitle("CONversation Player");
}

void SCConvPlayer::ReadNextFrame(void){
    
    
    
    
        
        if (conv.GetPosition() == end){
            Stop();
            return;
        }

        
        currentFrame.creationTime = SDL_GetTicks();
        
  
    
        uint8_t type = conv.ReadByte();
    
        switch (type) {
        case GROUP_SHOT:  // Group plan
        {
            char* location = (char*)conv.GetPosition();
            
            ConvBackGround* bg = ConvAssets.GetBackGround(location);
            
            
            currentFrame.mode = ConvFrame::CONV_WIDE;
            currentFrame.participants.clear();
            currentFrame.bgLayers = &bg->layers;
            currentFrame.bgPalettes = &bg->palettes;
            
            printf("ConvID: %d WIDEPLAN : LOCATION: '%s'\n",this->conversationID, location);
            conv.MoveForward(8+1);
            
            while(conv.PeekByte() == GROUP_SHOT_ADD_CHARCTER)
                ReadNextFrame();
            
            break;
        }
        case CLOSEUP:  // Person talking
        {
            char* speakerName =       (char*)conv.GetPosition();
            char* setName         = (char*)conv.GetPosition() + 0xA;
            char* sentence         = (char*)conv.GetPosition() + 0x17;
            
            
            currentFrame.mode = ConvFrame::CONV_CLOSEUP;
            currentFrame.face = ConvAssets.GetCharFace(speakerName);
            
            
            ConvBackGround* bg = ConvAssets.GetBackGround(setName);
            currentFrame.bgLayers = &bg->layers;
            currentFrame.bgPalettes = &bg->palettes;
            
            
            conv.MoveForward(0x17 + strlen((char*)sentence)+1);
            uint8_t color = conv.ReadByte(); // Color ?
            currentFrame.textColor = color;
            
            printf("ConvID: %d CLOSEUP: WHO: '%8s' WHERE: '%8s'     WHAT: '%s' (%2X)\n",this->conversationID,speakerName,setName,sentence,color);
            break;
        }
        case CLOSEUP_CONTINUATION:  // Same person keep talking
        {
            char* sentence         = (char*)conv.GetPosition();
            
            currentFrame.text = sentence;
            
            conv.MoveForward(strlen((char*)sentence)+1);
            printf("ConvID: %d MORETEX:                                       WHAT: '%s'\n",this->conversationID,sentence);
            break;
        }
        case YESNOCHOICE_BRANCH1:  // Choice Offsets are question
        {
            currentFrame.mode = ConvFrame::CONV_CONTRACT_CHOICE;
            printf("ConvID: %d CHOICE YES/NO : %X.\n",this->conversationID,type);
            //Looks like first byte is the offset to skip if the answer is no.
            /*uint8_t noOffset  =*/  conv.ReadByte();
            /*uint8_t yesOffset  =*/ conv.ReadByte();
            break;
        }
        case YESNOCHOICE_BRANCH2:  // Choice offset after first branch
        {
            
            //currentFrame.mode = ConvFrame::CONV_CONTRACT_CHOICE;
            printf("ConvID: %d CHOICE YES/NO : %X.\n",this->conversationID,type);
            //Looks like first byte is the offset to skip if the answer is no.
            /*uint8_t yesOffset  =*/ conv.ReadByte();
            /*uint8_t noOffset  =*/  conv.ReadByte();
            break;
        }
        case GROUP_SHOT_ADD_CHARCTER:  // Add person to GROUP
        {
            
            char* participantName  = (char*)conv.GetPosition();
            CharFigure* participant = ConvAssets.GetFigure(participantName);
            //currentFrame.participants.push_back(participant);
            
            printf("ConvID: %d WIDEPLAN ADD PARTICIPANT: '%s'\n",this->conversationID,conv.GetPosition());
            conv.MoveForward(0xD);
            break;
        }
        case GROUP_SHOT_CHARCTR_TALK:  // Make group character talk
        {
            
            char* who = (char*)conv.GetPosition();
            conv.MoveForward(0xE);
            char* sentence = (char*)conv.GetPosition();
            conv.MoveForward(strlen(sentence)+1);
            printf("ConvID: %d WIDEPLAN PARTICIPANT TALKING: who: '%s' WHAT '%s'\n",this->conversationID,who,sentence);
            
            
            break;
        }
        case SHOW_TEXT:  // Show text
        {
            int8_t color = conv.ReadByte();
            char* sentence = (char*)conv.GetPosition();
            
            currentFrame.text = sentence;
            currentFrame.textColor = color;
            
             printf("ConvID: %d Show Text: '%s' \n",this->conversationID,sentence);
            conv.MoveForward(strlen(sentence)+1);
            
            break;
        }
        case 0xE:
        {
            uint8_t unkn  = conv.ReadByte();
            uint8_t unkn1  = conv.ReadByte();
            printf("ConvID: %d Unknown usage Flag 0xE: (0x%2X 0x%2X) \n",this->conversationID,unkn,unkn1);
            ReadNextFrame();
            break;
        }
        case CHOOSE_WINGMAN:  // Wingman selection trigger
        {
            currentFrame.mode = ConvFrame::CONV_WINGMAN_CHOICE;
            printf("ConvID: %d Open pilot selection screen with current BG.\n",this->conversationID);
            break;
        }
        default:
            printf("ConvID: %d Unknown opcode: %X.\n",this->conversationID,type);
            Stop();
            return ;
            break;
        }
    
    
    
    this->currentFrame.SetExpired(false);
}

void SCConvPlayer::SetArchive(PakEntry* convPakEntry){
    
    if (convPakEntry->size == 0){
        Game.Log("Conversation entry is empty: Unable to load it.\n");
        Stop();
        return;
    }
    
    this->size = convPakEntry->size;
    
    this->conv.Set(convPakEntry->data);
    end = convPakEntry->data + convPakEntry->size;
    
    //Read a frame so we are ready to display it.
    //ReadNextFrame();
    this->currentFrame.SetExpired(true);
    
    initialized = true;
}


void SCConvPlayer::SetID(int32_t id){
    
    this->conversationID = id;
    
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
    VGAPalette* rendererPalette = VGA.GetPalette();
    this->palette = *rendererPalette;
}

void SCConvPlayer::CheckFrameExpired(void){
    
    //A frame expires either after a player press a key, click or 6 seconds elapse.
    //Mouse
    SDL_Event mouseEvents[5];
    int numMouseEvents= SDL_PeepEvents(mouseEvents,5,SDL_PEEKEVENT,SDL_MOUSEBUTTONUP,SDL_MOUSEBUTTONUP);
    for(int i= 0 ; i < numMouseEvents ; i++){
        SDL_Event* event = &mouseEvents[i];
        
        switch (event->type) {
            case SDL_MOUSEBUTTONUP:
                this->currentFrame.SetExpired(true);
                break;
            default:
                break;
        }
    }
    
    
    //Keyboard
    SDL_Event keybEvents[5];
    int numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_PEEKEVENT,SDL_KEYUP,SDL_KEYUP);
    for(int i= 0 ; i < numKeybEvents ; i++){
        SDL_Event* event = &keybEvents[i];
        switch (event->type) {
            default:
                this->currentFrame.SetExpired(true);
                break;
        }
    }
    
    /*
    int32_t currentTime = SDL_GetTicks();
    if(currentTime - currentFrame.creationTime > 5000)
        this->currentFrame.SetExpired(true);
    */
    
    
   
}


void SCConvPlayer::DrawText(const char* text, uint8_t type){
    
}



void SCConvPlayer::RunFrame(void){
    
    
    if (!initialized){
        Stop();
        Game.Log("Conv ID %d was not initialized: Stopping.\n", this->conversationID);
        return ;
    }
    
    //If frame needs to be update
    CheckFrameExpired();
    if ( currentFrame.IsExpired() )
        ReadNextFrame();
    
    
    CheckButtons();
    
    VGA.Activate();
    VGA.Clear();
    
    //Update the palette for the current background
    for (size_t i = 0; i < currentFrame.bgLayers->size(); i++) {
        ByteStream paletteReader;
        paletteReader.Set((*currentFrame.bgPalettes)[i]);
        this->palette.ReadPatch(&paletteReader);
        VGA.SetPalette(&this->palette);

    }
    
    
    //Draw static
    for (size_t i = 0; i < currentFrame.bgLayers->size(); i++) {
        VGA.DrawShape((*currentFrame.bgLayers)[i]);
    }

    
    //
    if (currentFrame.mode == ConvFrame::CONV_CLOSEUP ||
        currentFrame.mode == ConvFrame::CONV_CONTRACT_CHOICE){
        
        
        TreEntry* convPalettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVPALS.PAK");
        PakArchive convPals;
        convPals.InitFromRAM("CONVPALS.PAK", convPalettesEntry->data, convPalettesEntry->size);
        
        ByteStream paletteReader;
        paletteReader.Set(convPals.GetEntry(0x2E)->data); //mountains Good but not sky
        this->palette.ReadPatch(&paletteReader);
        
        //Face wiht of without hair
        //00 nothing
        //01 rest face
        //02 hair
        for (size_t i=1; i< 3; i++) {
           VGA.DrawShape(currentFrame.face->appearances->GetShape(i));
        }
        
        //Taking animation
        //03 mouth anim
        //04 mouth anim
        //06 mouth anim
        //07 mouth anim
        //08 mouth anim
        //09 mouth pinched
        //10 mouth opened
        //11 mouth something
        //12 mouth something
        
        for (size_t i=03; i< 11 && currentFrame.mode == ConvFrame::CONV_CLOSEUP
             ; i++) {
           VGA.DrawShape(currentFrame.face->appearances->GetShape(3+(SDL_GetTicks()/100)%10));
        }
        
        
        
        
        //Eyes animation
        //13 eyes closed
        //14 eyes closed
        //15 eyes wide open
        //16 eagle eyes
        //16 left wink
        //17 upper left eyes
        //18 look right
        //19 look left
        //20 eyes straight
        //21 eyes blink closed
        //22 eyes blink mid-open
        
        
        //23 eye brows semi-raised
        //24 left eye brows semi-raised
        //25 right eye brows semi-raised
        //26 eye brows something
        for (size_t i=13; i< 14; i++) {
            //VGA.DrawShape(currentFrame.face->appearances->GetShape(i));
        }
        
        //General face expression
        //27 mouth heart
        //28 face tensed
        //29 face smile
        //30 right face tensed
        //31 right crooked
        //32 pinched lips
        //33 surprise
        //34 seducing face
        //35 look of desaproval face
        for (size_t i=29; i< 30; i++) {
           //VGA.DrawShape(currentFrame.face->appearances->GetShape(i));
        }
        
        //Cloth
        //35 civil clothes
        //36 pilot clothes
        //37 pilot clothes 2
        //for (size_t i=36; i< 37; i++) {
            VGA.DrawShape(currentFrame.face->appearances->GetShape(35));
        //}
        
        //38 sunglasses
        //39 pilot helmet (if drawing this, don't draw hairs
        //40 pilot helmet visor (if drawing this draw 39 too
        for (size_t i=41; i< 40; i++) {
          VGA.DrawShape(currentFrame.face->appearances->GetShape(i));
        }
        
        //40 to 54 ????
        
        //54 hand extension
        if (currentFrame.mode == ConvFrame::CONV_CONTRACT_CHOICE){
            VGA.DrawShape(currentFrame.face->appearances->GetShape(54));
        }
        
        
        // 60 scary smile
        // 61 look right
        // 62 look left
        for (size_t i=55; i< 63; i++) {
            //What is there ?
            //VGA.DrawShape(currentFrame.face->appearances->GetShape(i));
        }
        
       
    }
    
    
    if (currentFrame.mode == ConvFrame::CONV_WIDE ||
        currentFrame.mode == ConvFrame::CONV_WINGMAN_CHOICE){
    
        for (size_t i=0 ; i < currentFrame.participants.size(); i++) {
            //CharFace* participant = currentFrame.participants[i];
            //      VGA.DrawShape(participant->appearance);
        }
    }
    
    //Draw text
    DrawText(currentFrame.text, currentFrame.textColor);
    
    DrawButtons();
    
    
    if (currentFrame.mode == ConvFrame::CONV_WIDE ||
        currentFrame.mode == ConvFrame::CONV_CLOSEUP)
        ;
    
    //Draw Mouse
    Mouse.Draw();
    
    //Check Mouse state.
    
    VGA.VSync();
    
    
}
