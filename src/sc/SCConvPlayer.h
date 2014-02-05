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
    
        RSFont* font;
        char* text;
        uint8_t textColor;
    
        enum ConvMode{ CONV_WIDE, CONV_CLOSEUP, CONV_WINGMAN_CHOICE, CONV_CONTRACT_CHOICE};
        ConvMode mode;
    
    //If we are in a wide of chose wingman mode
    std::vector<CharFigure*> participants;
    
    //If we are in close up mode
    CharFace* face;
    int8_t facePaletteID;
    
    std::vector<RLEShape*>* bgLayers;
    std::vector<uint8_t*> * bgPalettes;
    

    
    uint32_t creationTime; // Used to check when a frame expires.
    
    
    inline void SetExpired(bool exp){ this->expired = exp;}
    inline bool IsExpired(void){ return this->expired;}
        
    private:
        bool expired;
};


class SCConvPlayer: public IActivity {
    
public :
    SCConvPlayer();
    ~SCConvPlayer();
    
    void Init( );
    
    void RunFrame(void);
    
    void   SetID(int32_t id);
    
    
    virtual void Focus(void)  ;
   
    
private:
    
    int32_t conversationID;
    
    void ReadNextFrame(void);
    
    void    SetArchive(PakEntry* conv);
    
    void ReadtNextFrame(void);
    
    
    
    void DrawText(void);
    
    ByteStream conv ;
    size_t size; //In bytes
    uint8_t* end; //In bytes
    
    
    ConvFrame currentFrame;
    
    void CheckFrameExpired(void);
    
    
    bool initialized;
    
    
    
};

#endif /* defined(__libRealSpace__SCConvPlayer__) */
