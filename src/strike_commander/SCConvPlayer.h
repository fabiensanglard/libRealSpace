//
//  SCConvPlayer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCConvPlayer__
#define __libRealSpace__SCConvPlayer__
bool isNextFrameIsConv(uint8_t type);
class SCConvPlayer;
class ConvFrame {

public:
    RSFont *font{nullptr};
    char *text{nullptr};
    uint8_t textColor;
    uint8_t conversationID{0};
    enum ConvMode { CONV_WIDE, CONV_CLOSEUP, CONV_WINGMAN_CHOICE, CONV_CONTRACT_CHOICE, CONV_SHOW_TEXT };
    ConvMode mode;

    enum FacePos { FACE_DEF = 0x0, FACE_LEFT = 0x82, FACE_RIGHT = 0xBE, FACE_CENTER = 0xA0 };
    FacePos facePosition;
    uint8_t face_expression;
    // If we are in a wide of chose wingman mode
    std::vector<CharFigure *> participants;
    std::string *sound_file_name{nullptr};
    // If we are in close up mode
    CharFace *face{nullptr};
    int8_t facePaletteID;

    std::vector<RLEShape *> *bgLayers{nullptr};
    std::vector<uint8_t *> *bgPalettes{nullptr};

    uint32_t creationTime; // Used to check when a frame expires.

    inline void SetExpired(bool exp) { this->expired = exp; }
    inline bool IsExpired(void) { return this->expired; }
    void parse_GROUP_SHOT(ByteStream *conv);
    void parse_CLOSEUP(ByteStream *conv);
    void parse_CLOSEUP_CONTINUATION(ByteStream *conv);
    void parse_YESNOCHOICE_BRANCH1(ByteStream *conv, SCConvPlayer *player);
    void parse_YESNOCHOICE_BRANCH2(ByteStream *conv);
    void parse_GROUP_SHOT_ADD_CHARACTER(ByteStream *conv);
    void parse_GROUP_SHOT_CHARACTER_TALK(ByteStream *conv);
    void parse_SHOW_TEXT(ByteStream *conv);
    void parse_UNKNOWN(ByteStream *conv);
    void parse_CHOOSE_WINGMAN(ByteStream *conv, SCConvPlayer *player);
    int SetSentenceFromConv(ByteStream *conv, int start_offset);
private:
    bool expired;
};

class SCConvPlayer : public IActivity {
protected:
    int32_t conversationID;
    bool initialized;
    ByteStream conv;
    size_t size;  // In bytes
    uint8_t *end; // In bytes
    RSFont *font{nullptr};
    uint8_t topOffset{0};
    uint8_t first_palette{0};
    PakArchive convPak;
    PakArchive convPals;
    ConvFrame *current_frame{nullptr};

    virtual void CheckFrameExpired(void);
    void ReadNextFrame(void);
    void SetArchive(PakEntry *conv);
    void CheckZones(void);
    
    
    void DrawText(void);
    
public:
    uint8_t noOffset{0};
    uint8_t yesOffset{0};
    std::vector<SCZone *> zones;
    std::vector<ConvFrame *> conversation_frames;
    SCConvPlayer();
    ~SCConvPlayer();

    void init();
    void runFrame(void);
    void SetID(int32_t id);
    virtual void focus(void);
    void clicked(void *none, uint8_t id);
    void selectWingMan(void *none, uint8_t id);
};

#endif /* defined(__libRealSpace__SCConvPlayer__) */
