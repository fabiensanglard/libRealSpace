//
//  SCConvPlayer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCConvPlayer__
#define __libRealSpace__SCConvPlayer__




enum ConvMode{ WIDE, CLOSEUP};

class ConvFrame{
    
    public:
        uint8_t* convStart  ;
        uint8_t* convCurrent;
    
        char* text;
        uint8_t textColor;
    
        ConvMode mode;
    
        std::vector<NPCChar*> participants;
    
        RLEShape bg;
    
    uint32_t creationTime; // Used to check when a frame expires.
};


class SCConvPlayer: public IActivity {
    
public :
    SCConvPlayer();
    ~SCConvPlayer();
    
    void Init( );
    
    void RunFrame(void);
    
    void   SetID(int32_t id);
    
private:
    
    int32_t conversationID;
    
    void ReadNextFrame(void);
    
    void    SetArchive(PakEntry* conv);
    
    void ReadtNextFrame(void);
    
    
    
    
    
    ByteStream conv ;
    size_t size; //In bytes
    size_t read; //In bytes
    
    
    ConvFrame currentFrame;
    
    bool IsFrameExpired(void);
    
    
    
};

#endif /* defined(__libRealSpace__SCConvPlayer__) */
