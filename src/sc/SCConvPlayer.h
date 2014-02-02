//
//  SCConvPlayer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCConvPlayer__
#define __libRealSpace__SCConvPlayer__






class ConvFrame{
    
    public:
    
        char* text;
        uint8_t textColor;
    
        enum ConvMode{ CONV_WIDE, CONV_CLOSEUP};
        ConvMode mode;
    
        std::vector<NPCChar*> participants;
    
        RLEShape* bg;
    
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
    
    
    
    void DrawText(const char* text, uint8_t type);
    
    ByteStream conv ;
    size_t size; //In bytes
    size_t read; //In bytes
    
    
    ConvFrame currentFrame;
    
    bool IsFrameExpired(void);
    
    
    
};

#endif /* defined(__libRealSpace__SCConvPlayer__) */
