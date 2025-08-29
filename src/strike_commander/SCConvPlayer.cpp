//
//  SCConvPlayer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include "SCConvPlayer.h"

SCConvPlayer::SCConvPlayer() : conversationID(0), initialized(false) {}

SCConvPlayer::~SCConvPlayer() {}

#define GROUP_SHOT 0x00
#define GROUP_SHOT_ADD_CHARCTER 0x01
#define GROUP_SHOT_CHARCTR_TALK 0x02
#define CLOSEUP 0x03
#define CLOSEUP_CONTINUATION 0x04
#define SHOW_TEXT 0x0A
#define YESNOCHOICE_BRANCH1 0x0B
#define YESNOCHOICE_BRANCH2 0x0C
#define UNKNOWN 0x0E
#define CHOOSE_WINGMAN 0x0F

// Taking animation
// 03 mouth anim
// 04 mouth anim
// 06 mouth anim
// 07 mouth anim
// 08 mouth anim
// 09 mouth pinched
// 10 mouth opened
// 11 mouth something
// 12 mouth something

// Eyes animation
// 13 eyes closed
// 14 eyes closed
// 15 eyes wide open
// 16 eagle eyes
// 16 left wink
// 17 upper left eyes
// 18 look right
// 19 look left
// 20 eyes straight
// 21 eyes blink closed
// 22 eyes blink mid-open

// 23 eye brows semi-raised
// 24 left eye brows semi-raised
// 25 right eye brows semi-raised
// 26 eye brows something

// General face expression
// 27 mouth heart
// 28 face tensed
// 29 face smile
// 30 right face tensed
// 31 right crooked
// 32 pinched lips
// 33 surprise
// 34 seducing face
// 35 look of desaproval face

// Cloth
// 35 civil clothes
// 36 pilot clothes
// 37 pilot clothes 2

// 38 sunglasses
// 39 pilot helmet (if drawing this, don't draw hairs
// 40 pilot helmet visor (if drawing this draw 39 too

std::map<uint8_t, std::vector<uint8_t>> faces_shape = {
    { 13,     std::vector<uint8_t>{1, 14, 2, 35}},
    { 14, std::vector<uint8_t>{1, 21, 28, 2, 35}},
    { 16,     std::vector<uint8_t>{1, 17, 2, 35}},
    { 17,     std::vector<uint8_t>{1, 18, 2, 35}},
    { 18,     std::vector<uint8_t>{1, 19, 2, 35}},
    { 19,         std::vector<uint8_t>{1, 2, 35}},
    { 20, std::vector<uint8_t>{1, 21, 28, 2, 35}},
    { 21, std::vector<uint8_t>{1, 21, 28, 2, 35}}, //todo check
    { 22,     std::vector<uint8_t>{1, 23, 2, 35}},
    { 23,     std::vector<uint8_t>{1, 24, 2, 35}},
    { 24,     std::vector<uint8_t>{1, 25, 2, 35}},
    { 25,     std::vector<uint8_t>{1, 26, 2, 35}},
    { 26,     std::vector<uint8_t>{1, 26, 2, 35}}, //todo check
    { 27,     std::vector<uint8_t>{1, 27, 2, 35}}, //todo check
    { 28,     std::vector<uint8_t>{1, 28, 2, 35}}, //todo check
    { 29,     std::vector<uint8_t>{1, 29, 2, 35}}, //todo check
    { 31,     std::vector<uint8_t>{1, 31, 2, 35}},
    { 32,     std::vector<uint8_t>{1, 33, 2, 35}}, //todo check
    {255,         std::vector<uint8_t>{1, 2, 35}},
};

bool isNextFrameIsConv(uint8_t type) {
    switch (type) {
        case GROUP_SHOT:
        case GROUP_SHOT_ADD_CHARCTER:
        case GROUP_SHOT_CHARCTR_TALK:
        case CLOSEUP:
        case CLOSEUP_CONTINUATION:
        case SHOW_TEXT:
        case YESNOCHOICE_BRANCH1:
        case YESNOCHOICE_BRANCH2:
        case CHOOSE_WINGMAN:
        case 0xE0:
            return true;
        default:
            return false;
    }
    return false;
}

void ConvFrame::parse_GROUP_SHOT(ByteStream *conv) {
    char *location = (char *)conv->GetPosition();

    ConvBackGround *bg = ConvAssets.GetBackGround(location);

    this->mode = ConvFrame::CONV_WIDE;
    this->participants.clear();
    this->participants.shrink_to_fit();
    this->bgLayers   = &bg->layers;
    this->bgPalettes = &bg->palettes;
    this->face      = nullptr;
    printf("ConvID: %d WIDEPLAN : LOCATION: '%s'\n", this->conversationID, location);
    conv->MoveForward(8);
    while (conv->PeekByte() == 0x0) {
        conv->MoveForward(1);
    }
    while (!isNextFrameIsConv((uint8_t) conv->PeekByte())) {
        conv->MoveForward(1);
    }
    uint8_t next_type = conv->PeekByte();
    if (next_type == GROUP_SHOT_ADD_CHARCTER) {
        conv->MoveForward(2);
        this->parse_GROUP_SHOT_ADD_CHARACTER(conv);
    } else if (next_type == SHOW_TEXT) {
        conv->MoveForward(2);
        this->parse_SHOW_TEXT(conv);
    } else if (next_type == GROUP_SHOT_CHARCTR_TALK) {
        conv->MoveForward(2);
        this->parse_GROUP_SHOT_CHARACTER_TALK(conv);
    } else {
        conv->MoveForward(1);
    }
}
void ConvFrame::parse_CLOSEUP(ByteStream *conv) {
    char *speakerName            = (char *)conv->GetPosition();
    this->face_expression = (uint8_t)*(conv->GetPosition() + 0x09);
    char *setName                = (char *)conv->GetPosition() + 0xA;

    int next_frame_offset = SetSentenceFromConv(conv,0x17);
    this->participants.clear();
    this->participants.shrink_to_fit();
    uint8_t pos               = *(conv->GetPosition() + 0x13);
    this->facePosition = static_cast<ConvFrame::FacePos>(pos);

    this->mode         = ConvFrame::CONV_CLOSEUP;
    this->face         = ConvAssets.GetCharFace(speakerName);
    ConvBackGround *bg        = ConvAssets.GetBackGround(setName);
    this->bgLayers     = &bg->layers;
    this->bgPalettes   = &bg->palettes;

    conv->MoveForward(next_frame_offset);
    uint8_t color              = conv->ReadByte(); // Color ?
    this->textColor            = color;
    const char *pszExt         = "normal";
    this->facePaletteID = ConvAssets.GetFacePaletteID(const_cast<char *>(pszExt));

    printf(
        "ConvID: %d CLOSEUP: WHO: '%8s' WHERE: '%8s'     WHAT: '%s' (%2X) pos %2X  face expression: '%d'\n",
        this->conversationID, speakerName, setName, this->text, color, pos, this->face_expression
    );
}
void ConvFrame::parse_CLOSEUP_CONTINUATION(ByteStream *conv) {
    int next_frame_offset = SetSentenceFromConv(conv,0x0);
    conv->MoveForward(next_frame_offset);
    printf(
        "ConvID: %d MORETEX:                                       WHAT: '%s'\n", this->conversationID, this->text
    );
}
void ConvFrame::parse_YESNOCHOICE_BRANCH1(ByteStream *conv, SCConvPlayer *player) {
    this->mode = ConvFrame::CONV_CONTRACT_CHOICE;
    // Looks like first byte is the offset to skip if the answer is no.
    uint8_t noOffset  = conv->ReadByte();
    uint8_t yesOffset = conv->ReadByte();
    printf("Offsets: %d %d\n", noOffset, yesOffset);
    SCZone *zone = new SCZone();
    zone->label  = new std::string("yes");
    zone->quad   = new std::vector<Point2D *>();
    Point2D *p1  = new Point2D{0, 100};
    Point2D *p2  = new Point2D{120, 100};
    Point2D *p3  = new Point2D{120, 199};
    Point2D *p4  = new Point2D{0, 199};

    zone->id = 1;
    zone->quad->push_back(p1);
    zone->quad->push_back(p2);
    zone->quad->push_back(p3);
    zone->quad->push_back(p4);
    zone->onclick = std::bind(&SCConvPlayer::clicked, player, std::placeholders::_1, std::placeholders::_2);
    player->zones.push_back(zone);

    zone        = new SCZone();
    zone->id    = 2;
    zone->label = new std::string("no");
    zone->quad  = new std::vector<Point2D *>();

    Point2D *np1 = new Point2D{200, 100};
    Point2D *np2 = new Point2D{320, 100};
    Point2D *np3 = new Point2D{320, 199};
    Point2D *np4 = new Point2D{200, 199};

    zone->quad->push_back(np1);
    zone->quad->push_back(np2);
    zone->quad->push_back(np3);
    zone->quad->push_back(np4);
    zone->onclick = std::bind(&SCConvPlayer::clicked, player, std::placeholders::_1, std::placeholders::_2);
    player->zones.push_back(zone);
    this->yes_no_path = 1;
}
void ConvFrame::parse_YESNOCHOICE_BRANCH2(ByteStream *conv) {
    uint8_t t = conv->ReadByte();
    uint8_t b = conv->ReadByte();
    this->yes_no_path = 2;
}
void ConvFrame::parse_GROUP_SHOT_ADD_CHARACTER(ByteStream *conv) {
    char *participantName   = (char *)conv->GetPosition();
    CharFigure *participant = ConvAssets.GetFigure(participantName);
    

    printf("ConvID: %d WIDEPLAN ADD PARTICIPANT: '%s'\n", this->conversationID, conv->GetPosition());
    conv->MoveForward(8);
    participant->x = conv->ReadUShortBE();
    participant->y = conv->ReadUShortBE();
    this->participants.push_back(participant);
    while (conv->PeekByte() == 0x0) {
        conv->MoveForward(1);
    }
    while (!isNextFrameIsConv((uint8_t) conv->PeekByte())) {
        conv->MoveForward(1);
    }
    if (conv->PeekByte() == GROUP_SHOT_ADD_CHARCTER) {
        conv->MoveForward(2);
        this->parse_GROUP_SHOT_ADD_CHARACTER(conv);
    } else if (conv->PeekByte() == SHOW_TEXT) {
        conv->MoveForward(2);
        this->parse_SHOW_TEXT(conv);
    } else {
        conv->MoveForward(1);
    }
}
void ConvFrame::parse_GROUP_SHOT_CHARACTER_TALK(ByteStream *conv) {
    char *who = (char *)conv->GetPosition();
    int next_frame_offset = this->SetSentenceFromConv(conv, 0xE);
    conv->MoveForward(next_frame_offset);
    printf("ConvID: %d WIDEPLAN PARTICIPANT TALKING: who: '%s' WHAT '%s'\n", this->conversationID, who, this->text);
    CharFigure *participant = ConvAssets.GetFigure(who);
    this->participants.push_back(participant);
}
void ConvFrame::parse_SHOW_TEXT(ByteStream *conv) {
    this->mode = ConvFrame::CONV_SHOW_TEXT;
    int8_t color      = conv->ReadByte();
    int next_frame_offset = this->SetSentenceFromConv(conv, 0);
    this->textColor = color;
    printf("ConvID: %d Show Text: '%s' \n", this->conversationID, this->text);
    conv->MoveForward(next_frame_offset);
    conv->PeekByte();
}
void ConvFrame::parse_UNKNOWN(ByteStream *conv) {
}
void ConvFrame::parse_CHOOSE_WINGMAN(ByteStream *conv, SCConvPlayer *player) {
    ConvBackGround *bg = ConvAssets.GetBackGround(const_cast<char*>("s_wing"));

    this->bgLayers   = &bg->layers;
    this->bgPalettes = &bg->palettes;

    this->participants.clear();
    this->mode = ConvFrame::CONV_WINGMAN_CHOICE;
    printf("ConvID: %d Open pilot selection screen with current BG.\n", this->conversationID);
    CharFigure *entry = ConvAssets.GetFigure(const_cast<char*>("red1"));
    this->participants.push_back(entry);
    std::vector<std::string> airwing = {"billy2", "gwen2", "lyle2", "miguel2", "tex3"};
    std::vector<std::string> airwing_names = {"Lyle", "Miguel", "Gwen", "Billy", "Tex"};
    std::vector<std::vector<Point2D>> zonespos = {
        { { 0, 80 }, { 60, 80 }, { 60, 200 }, { 0, 200 } },
        { { 64, 80 }, { 110, 80 }, { 110, 200 }, { 64, 200 } },
        { { 120, 80 }, { 170, 80 }, { 170, 200 }, { 120, 200 } },
        { { 175, 80 }, { 218, 80 }, { 218, 200 }, { 175, 200 } },
        { { 228, 80 }, { 268, 80 }, { 268, 200 }, { 228, 200 } }
    };
    player->zones.clear();
    player->zones.shrink_to_fit();
    int i = 0;
    for (auto pilot : airwing) {
        CharFigure *participant = ConvAssets.GetFigure((char *)pilot.c_str());
        this->participants.push_back(participant);
        SCZone *zone = new SCZone();
        zone->label  = new std::string("Select "+airwing_names[i]);
        zone->quad   = new std::vector<Point2D *>();
        for (auto zp: zonespos[i]) {
            Point2D *p = new Point2D{zp.x, zp.y};
            zone->quad->push_back(p);
        }
        zone->id = i;
        zone->active = true;
        zone->onclick = std::bind(&SCConvPlayer::selectWingMan, player, std::placeholders::_1, std::placeholders::_2);
        player->zones.push_back(zone);
        i++;
    }
}

void SCConvPlayer::focus(void) { IActivity::focus(); }

void SCConvPlayer::clicked(void *none, uint8_t id) {
    printf("clicked on %d\n", id);
    if (id != 1 && id != 2) {
        this->current_frame->SetExpired(true);
        return;
    }

    bool accepted = (id == 1);
    GameState.mission_accepted = accepted;

    // On veut supprimer les frames de la branche opposée :
    // si accepté -> supprimer yes_no_path == 2 (branche NO)
    // si refusé  -> supprimer yes_no_path == 1 (branche YES)
    uint8_t pathToRemove = accepted ? 2 : 1;

    // Ne pas toucher à current_frame (première frame affichée de la question).
    // Parcours sûr avec itérateur, suppression in situ.
    // On part après current_frame si elle est bien en tête.
    auto it = conversation_frames.begin();
    if (it != conversation_frames.end() && *it == this->current_frame) {
        ++it;
    }

    while (it != conversation_frames.end()) {
        ConvFrame *f = *it;
        if (f && f->yes_no_path == pathToRemove) {
            delete f;                                   // libérer la mémoire
            it = conversation_frames.erase(it);        // erase retourne le suivant
        } else {
            ++it;
        }
    }
    // Optionnel: compaction après coup seulement (évite realloc pendant la boucle)
    conversation_frames.shrink_to_fit();

    this->current_frame->SetExpired(true);
}
void SCConvPlayer::selectWingMan(void *none, uint8_t id) {
    printf("clicked on %d\n", id);
    std::map<uint8_t, std::string> wingman_map = {
        {0,"LYLE"},
        {1,"MIGUEL"},
        {2,"GWEN"},
        {3,"BILLY"},
        {4,"TEX"}
    };
    if (wingman_map.find(id) == wingman_map.end()) {
        return;
    } else {
        GameState.wingman = wingman_map[id];
        this->current_frame->SetExpired(true);
    }
    
}

int ConvFrame::SetSentenceFromConv(ByteStream *conv, int start_offset) {
    char *sentence = (char *)conv->GetPosition() + start_offset;
    int sound_offset = 0;
    char *sentence_end = (char *)conv->GetPosition() + start_offset + strlen((char *)sentence) + 1;
    this->sound_file_name = nullptr;
    int decalage = 1;
    if ((int8_t)sentence_end[0] < 0) {
        sentence_end += 1;
        decalage = 2;
    }
    if (!isNextFrameIsConv((uint8_t) sentence_end[0])) {
        sound_offset = (int) strlen((char *)sentence) + decalage;
        this->sound_file_name = new std::string(sentence);
        sentence = sentence_end;
    }
    std::string *text = new std::string(sentence);
    if (text->find("$N") != std::string::npos) {
        text->replace(text->find("$N"), 2, GameState.player_firstname);
    }
    if (text->find("$S") != std::string::npos) {
        text->replace(text->find("$S"), 2, GameState.player_name);
    }
    if (text->find("$C") != std::string::npos) {
        text->replace(text->find("$C"), 2, GameState.player_callsign);
    }
    if (text->find("$W") != std::string::npos) {
        text->replace(text->find("$W"), 2, GameState.wingman);
    }
    this->text         = (char *)text->c_str();
    return (int) (sound_offset + start_offset + strlen((char *)sentence) + 1);
}
/**
 * @brief Read the next frame of the conversation from the data stream.
 *
 * If the end of the conversation is reached, stop the activity.
 *
 * The function reads the next byte from the stream and uses it to determine the type of frame to read.
 * Depending on the type of frame, it reads the appropriate data from the stream and stores it in tmp_frame->
 * If the type of frame is unknown, it stops the activity and prints an error message.
 */
void SCConvPlayer::ReadNextFrame(void) {

    ConvFrame *tmp_frame = new ConvFrame();

    tmp_frame->creationTime       = SDL_GetTicks();
    tmp_frame->text               = nullptr;
    tmp_frame->sound_file_name    = nullptr;
    tmp_frame->facePaletteID      = 0;
    tmp_frame->font = this->font;
    tmp_frame->conversationID = this->conversationID;
    // tmp_frame->face = nullptr;

    this->parseConv(tmp_frame);

    tmp_frame->SetExpired(false);
    if (this->conversation_frames.size() > 0) {
        if (tmp_frame->bgLayers == nullptr) {
            tmp_frame->bgLayers = this->conversation_frames.back()->bgLayers;
            tmp_frame->bgPalettes = this->conversation_frames.back()->bgPalettes;
        }
    }
    if (this->txt_color != 0 && tmp_frame->textColor == 0) {
        tmp_frame->textColor = this->txt_color;
    } else if (tmp_frame->textColor != 0) {
        this->txt_color = tmp_frame->textColor;
    }
    if (this->yes_no_path == 0 && tmp_frame->yes_no_path != 0) {
        this->yes_no_path = tmp_frame->yes_no_path;
    } else if (this->yes_no_path == 1 && tmp_frame->yes_no_path != 0) {
        this->yes_no_path = tmp_frame->yes_no_path;
    } else if (tmp_frame->yes_no_path == 0) {
        tmp_frame->yes_no_path = this->yes_no_path;
    }
    if (!tmp_frame->do_not_add) {
        this->conversation_frames.push_back(tmp_frame);
    } else {
        delete tmp_frame;
    }
    
}

void SCConvPlayer::parseConv(ConvFrame *tmp_frame) {
    uint8_t type = conv.ReadByte();

    switch (type) {
    case GROUP_SHOT: // Group plan
    {
        tmp_frame->parse_GROUP_SHOT(&conv);
        break;
    }
    case CLOSEUP: // Person talking
    {
        tmp_frame->parse_CLOSEUP(&conv);
        break;
    }
    case CLOSEUP_CONTINUATION: // Same person keep talking
    {
        tmp_frame->parse_CLOSEUP_CONTINUATION(&conv);
        tmp_frame->textColor = this->conversation_frames.back()->textColor;
        tmp_frame->face = this->conversation_frames.back()->face;
        tmp_frame->face_expression = this->conversation_frames.back()->face_expression;
        tmp_frame->mode = this->conversation_frames.back()->mode;
        tmp_frame->facePaletteID = this->conversation_frames.back()->facePaletteID;
        tmp_frame->facePosition = this->conversation_frames.back()->facePosition;
        break;
    }
    case YESNOCHOICE_BRANCH1: // Choice Offsets are question
    {
        printf("ConvID: %d CHOICE YES/NO : %d\n", this->conversationID, type);
        tmp_frame->parse_YESNOCHOICE_BRANCH1(&conv, this);
        tmp_frame->face = this->conversation_frames.back()->face;
        tmp_frame->textColor = this->conversation_frames.back()->textColor;
        tmp_frame->face_expression = this->conversation_frames.back()->face_expression;
        tmp_frame->facePaletteID = this->conversation_frames.back()->facePaletteID;
        tmp_frame->facePosition = this->conversation_frames.back()->facePosition;
        break;
    }
    case YESNOCHOICE_BRANCH2: // Choice offset after first branch
    {

        // tmp_frame->mode = ConvFrame::CONV_CONTRACT_CHOICE;
        printf("ConvID: %d CHOICE YES/NO : %d\n", this->conversationID, type);
        // Looks like first byte is the offset to skip if the answer is no.
        tmp_frame->parse_YESNOCHOICE_BRANCH2(&conv);
        tmp_frame->do_not_add = true;
        break;
    }
    case GROUP_SHOT_ADD_CHARCTER: // Add person to GROUP
    {
        tmp_frame->parse_GROUP_SHOT_ADD_CHARACTER(&conv);
        break;
    }
    case GROUP_SHOT_CHARCTR_TALK: // Make group character talk
    {
        tmp_frame->parse_GROUP_SHOT_CHARACTER_TALK(&conv);  
        break;
    }
    case SHOW_TEXT: // Show text
    {
        tmp_frame->parse_SHOW_TEXT(&conv);
        tmp_frame->face = this->conversation_frames.back()->face;
        tmp_frame->textColor = this->conversation_frames.back()->textColor;
        tmp_frame->face_expression = this->conversation_frames.back()->face_expression;
        tmp_frame->facePaletteID = this->conversation_frames.back()->facePaletteID;
        tmp_frame->facePosition = this->conversation_frames.back()->facePosition;
        
        break;
    }
    case 0xE: {
        topOffset = conv.ReadByte();
        first_palette = conv.ReadByte();
        this->parseConv(tmp_frame);
        printf("ConvID: %d Unknown usage Flag 0xE: (0x%2X 0x%2X) \n", this->conversationID, topOffset, first_palette);
        break;
    }
    case CHOOSE_WINGMAN: // Wingman selection trigger
    {
        tmp_frame->parse_CHOOSE_WINGMAN(&conv, this);
        break;
    }
    default:
        printf("ConvID: %d Unknown opcode: %X.\n", this->conversationID, type);
        tmp_frame->do_not_add = true;
        break;
    }
}
void SCConvPlayer::SetArchive(PakEntry *convPakEntry) {

    if (convPakEntry->size == 0) {
        Game->log("Conversation entry is empty: Unable to load it.\n");
        stop();
        return;
    }

    this->size = convPakEntry->size;

    this->conv.Set(convPakEntry->data);
    end = convPakEntry->data + convPakEntry->size;

    if (this->conversation_frames.size() > 0) {
        for (auto frame : this->conversation_frames) {
            delete frame;
        }
        this->conversation_frames.clear();
        this->conversation_frames.shrink_to_fit();
    }

    while (conv.GetPosition() < end) {
        ReadNextFrame();
    }

    initialized = true;
}

void SCConvPlayer::SetID(int32_t id) {

    this->conversationID = id;

    if (convPak.GetNumEntries() <= id) {
        stop();
        Game->log("Cannot load conversation id (max convID is %lu).", convPak.GetNumEntries() - 1);
        return;
    }
    topOffset = CONV_TOP_BAR_HEIGHT + 1;
    SetArchive(convPak.GetEntry(id));
}

void SCConvPlayer::init() {
    VGAPalette *rendererPalette = VGA.GetPalette();
    this->palette               = *rendererPalette;
    TreEntry *convEntry = Assets.GetEntryByName(Assets.convpak_filename);

    convPak.InitFromRAM("CONV.PAK", convEntry->data, convEntry->size);
    this->font = FontManager.GetFont("..\\..\\DATA\\FONTS\\CONVFONT.SHP");

    TreEntry *convPalettesEntry = Assets.GetEntryByName(Assets.conv_pal_filename);
    convPals.InitFromRAM("CONVPALS.PAK", convPalettesEntry->data, convPalettesEntry->size);
}

/**
 * @brief Check if the current frame has expired, either because of user input or
 *        because a certain amount of time has passed.
 *
 * If the user has pressed a key or clicked, the current frame is marked as
 * expired. If the frame has been displayed for longer than 5 seconds, the
 * current frame is also marked as expired.
 */
void SCConvPlayer::CheckFrameExpired(void) {
    if (this->current_frame->mode != ConvFrame::CONV_CONTRACT_CHOICE && this->current_frame->mode != ConvFrame::CONV_WINGMAN_CHOICE) {
        if (!Game)
        return;
        Keyboard* kb = Game->getKeyboard();
        if (!kb)
            return;
        if (kb->isActionJustPressed(InputAction::KEY_ESCAPE)) {
            Game->stopTopActivity();
            Mixer.StopSound();
        }
        if (kb->isActionJustPressed(InputAction::MOUSE_LEFT)) {
            this->current_frame->SetExpired(true);
            Mixer.StopSound();
            return;
        }

    
        int32_t currentTime = SDL_GetTicks();

        if (currentTime - this->current_frame->creationTime > 5000)
            this->current_frame->SetExpired(true);
            Mixer.StopSound();
    }
}

/**
 * @brief Draw a block of text within a conv window.
 *
 * @details
 * This function is used by the conversation engine to draw a block of text
 * within a conversation window. The text is split into lines. The width of
 * each line is determined by the number of characters that can fit between
 * the left and right border of the conversation window. If a word does not fit
 * in a line, it is moved to the next line. The text is centered.
 *
 * @param[in]   text        The text to draw.
 * @param[in]   textColor   The color of the text.
 * @param[in]   font        The font use to draw the text.
 */
void SCConvPlayer::DrawText(void) {

    if (this->current_frame->text == NULL)
        return;

    size_t textSize    = strlen(this->current_frame->text);
    const char *cursor = this->current_frame->text;
    const char *end    = cursor + textSize;

    uint8_t lineNumber = 0;

    while (cursor < end) {

        const char *wordSearch  = cursor;
        const char *lastGoodPos = wordSearch;

        // How many pixels are avaiable for a line.
        int32_t pixelAvailable = 320 - CONV_BORDER_MARGIN * 2;

        // Determine what will fit in a line.
        while (pixelAvailable > 0 && wordSearch < end) {

            lastGoodPos = wordSearch - 1;
            while (*wordSearch != ' ' && wordSearch < end) {

                if (*wordSearch == ' ') {
                    pixelAvailable -= CONV_SPACE_SIZE;
                } else {
                    RLEShape *letter = this->current_frame->font->GetShapeForChar(*wordSearch);
                    if (letter != nullptr) {
                        pixelAvailable -= letter->GetWidth() + CONV_INTERLETTER_SPACE;
                    } else {
                        pixelAvailable -= 8 + CONV_INTERLETTER_SPACE;
                    }
                }
                wordSearch++;
            }

            if (pixelAvailable > 0)
                lastGoodPos = this->current_frame->text + strlen(this->current_frame->text);
            // Skip the space char
            wordSearch++;
        }

        // Draw the line
        Point2D coo = {CONV_BORDER_MARGIN, 165 + lineNumber * 13};

        if (pixelAvailable < 0)
            pixelAvailable = 0;
        // Don't forget to center the text
        coo.x += pixelAvailable / 2;

        VGA.GetFrameBuffer()->PrintText(
            this->font, &coo, this->current_frame->text, static_cast<uint32_t>(this->current_frame->textColor),
            static_cast<uint32_t>(cursor - this->current_frame->text), static_cast<uint32_t>(lastGoodPos - cursor),
            CONV_INTERLETTER_SPACE, CONV_SPACE_SIZE
        );

        // Go to next line
        cursor = lastGoodPos + 1;
        lineNumber++;
    }
}
void SCConvPlayer::CheckZones() {
    for (auto zone : zones) {
        if (zone->IsActive(&GameState.requierd_flags)) {
            if (zone->quad != nullptr) {
                if (isPointInQuad(Mouse.GetPosition(), zone->quad)) {
                    Mouse.SetMode(SCMouse::VISOR);

                    // If the mouse button has just been released: trigger action.
                    if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::RELEASED)
                        zone->OnAction();
                    Point2D p = {160 - static_cast<int32_t>(zone->label->length() / 2) * 8, 180};
                    VGA.GetFrameBuffer()->PrintText(
                        FontManager.GetFont("..\\..\\DATA\\FONTS\\OPTFONT.SHP"), &p, (char *)zone->label->c_str(), 64, 0,
                        static_cast<uint32_t>(zone->label->length()), 3, 5
                    );
                    return;
                }
            }
            if (Mouse.GetPosition().x > zone->position.x &&
                Mouse.GetPosition().x < zone->position.x + zone->dimension.x &&
                Mouse.GetPosition().y > zone->position.y &&
                Mouse.GetPosition().y < zone->position.y + zone->dimension.y) {
                // HIT !
                Mouse.SetMode(SCMouse::VISOR);

                // If the mouse button has just been released: trigger action.
                if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::RELEASED)
                    zone->OnAction();
                Point2D p = {160 - ((int32_t)(zone->label->length() / 2) * 8), 180};
                VGA.GetFrameBuffer()->PrintText(
                    FontManager.GetFont(""), &p, (char *)zone->label->c_str(), 64, 0,
                    static_cast<uint32_t>(zone->label->length()), 3, 5
                );

                return;
            }
        }
    }
}
/**
 * Runs a single frame of the conversation player, updating the game state and rendering the graphics.
 *
 * This function is called by the game engine to update the state of the conversation player and to
 * render the graphics. It checks if the current frame needs to be updated and if so, reads the next
 * frame from the conversation data. It also checks if the buttons need to be drawn and if so, draws
 * them.
 *
 * @param None
 *
 * @return None
 *
 */
void SCConvPlayer::runFrame(void) {

    if (!initialized) {
        stop();
        Game->log("Conv ID %d was not initialized: Stopping.\n", this->conversationID);
        return;
    }
    if (this->conversation_frames.size() == 0) {
        stop();
        Game->log("Conv ID %d has no frames: Stopping.\n", this->conversationID);
        return;
    }
    if (this->current_frame == nullptr) {
        this->current_frame = this->conversation_frames[0];
        this->current_frame->creationTime       = SDL_GetTicks();
    }
    // If frame needs to be update
    if (this->current_frame->mode != ConvFrame::CONV_CONTRACT_CHOICE && this->current_frame->mode != ConvFrame::CONV_WINGMAN_CHOICE) {
        CheckFrameExpired();
    }

    if (this->current_frame->IsExpired()) {
        this->conversation_frames.erase(this->conversation_frames.begin());
        this->conversation_frames.shrink_to_fit();
        if (this->conversation_frames.size() == 0) {
            stop();
            Game->log("Conv ID %d has no frames: Stopping.\n", this->conversationID);
            return;
        }
        this->current_frame = this->conversation_frames[0];
        this->current_frame->creationTime       = SDL_GetTicks();
    }
    
    checkButtons();

    VGA.Activate();
    VGA.GetFrameBuffer()->FillWithColor(255);
    // Update the palette for the current background
    if (this->current_frame->bgLayers != nullptr && this->current_frame->bgPalettes != nullptr) {
        for (size_t i = 0; i < this->current_frame->bgLayers->size(); i++) {
            ByteStream paletteReader;
            paletteReader.Set((*this->current_frame->bgPalettes)[i]);
            this->palette.ReadPatch(&paletteReader);
        }
        VGA.SetPalette(&this->palette);
        for (size_t i = 0; i < this->current_frame->bgLayers->size(); i++) {
            RLEShape *shape = (*this->current_frame->bgLayers)[i];
            VGA.GetFrameBuffer()->DrawShape(shape);
        }
    }
    
    
    // Draw static
    
    if (this->current_frame->sound_file_name != NULL && this->current_frame->sound_file_name->length() > 0) {
        std::string filename = "F:"+*this->current_frame->sound_file_name+".VOC";
        std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);
        TreEntry *sound_entry = Assets.GetEntryByName(filename.c_str());
        if (sound_entry != nullptr) {
            Mixer.PlaySoundVoc(sound_entry->data, sound_entry->size);
        }
    }

    switch (this->current_frame->mode) {
        case ConvFrame::CONV_SHOW_TEXT:
        {
            for (size_t i = 0; i < CONV_TOP_BAR_HEIGHT + 1; i++)
            VGA.GetFrameBuffer()->FillLineColor(i, 0x00);

            for (size_t i = 0; i < CONV_BOTTOM_BAR_HEIGHT; i++)
                VGA.GetFrameBuffer()->FillLineColor(199 - i, 0x00);
            ByteStream paletteReader;

            paletteReader.Set(convPals.GetEntry(this->current_frame->facePaletteID)->data);
            this->palette.ReadPatch(&paletteReader);

            int32_t pos = 0;

            if (this->current_frame->facePosition == ConvFrame::FACE_LEFT)
                pos = -30;

            if (this->current_frame->facePosition == ConvFrame::FACE_RIGHT)
                pos = 30;

            if (this->current_frame->face != NULL) {
                RLEShape *s = nullptr;
                if (faces_shape.find(this->current_frame->face_expression) != faces_shape.end()) {
                    for (auto shape : faces_shape[this->current_frame->face_expression]) {
                        s = this->current_frame->face->appearances->GetShape(shape);
                        s->SetPositionX(pos);
                        VGA.GetFrameBuffer()->DrawShape(s);
                    }
                }
            } else {
                for (size_t i = 0; i < this->current_frame->participants.size(); i++) {
                    CharFigure *participant = this->current_frame->participants[i];
                    RLEShape *s = participant->appearances->GetShape(0);
                    Point2D position = {participant->x, participant->y};
                    s->SetPosition(&position);
                    VGA.GetFrameBuffer()->DrawShape(s);
                    if (participant->appearances->GetNumImages() > 1) {
                        s = participant->appearances->GetShape(1);
                        s->SetPosition(&position);
                        VGA.GetFrameBuffer()->DrawShape(s);
                    }
                    position.x += s->GetWidth();
                }
            }
            for (size_t i = 0; i < CONV_TOP_BAR_HEIGHT; i++)
                VGA.GetFrameBuffer()->FillLineColor(i, 0x00);

            for (size_t i = 0; i < CONV_BOTTOM_BAR_HEIGHT; i++)
                VGA.GetFrameBuffer()->FillLineColor(199 - i, 0x00);

        }
        break;
        case ConvFrame::CONV_CLOSEUP:
        {
            for (size_t i = 0; i < CONV_TOP_BAR_HEIGHT + 1; i++)
            VGA.GetFrameBuffer()->FillLineColor(i, 0x00);

            for (size_t i = 0; i < CONV_BOTTOM_BAR_HEIGHT; i++)
                VGA.GetFrameBuffer()->FillLineColor(199 - i, 0x00);
            ByteStream paletteReader;

            paletteReader.Set(convPals.GetEntry(this->current_frame->facePaletteID)->data);
            this->palette.ReadPatch(&paletteReader);

            int32_t pos = 0;

            if (this->current_frame->mode == ConvFrame::CONV_CLOSEUP) {
                if (this->current_frame->facePosition == ConvFrame::FACE_LEFT)
                    pos = -30;

                if (this->current_frame->facePosition == ConvFrame::FACE_RIGHT)
                    pos = 30;
            }

            if (this->current_frame->face != NULL) {
                RLEShape *s = nullptr;
                if (faces_shape.find(this->current_frame->face_expression) != faces_shape.end()) {
                    for (auto shape : faces_shape[this->current_frame->face_expression]) {
                        s = this->current_frame->face->appearances->GetShape(shape);
                        s->SetPositionX(pos);
                        VGA.GetFrameBuffer()->DrawShape(s);
                    }
                }
                for (size_t i = 03; i < 11 && this->current_frame->text != nullptr && this->current_frame->text[0] != '\0'; i++) {
                    RLEShape *s = this->current_frame->face->appearances->GetShape(3 + (SDL_GetTicks() / 100) % 10);
                    s->SetPositionX(pos);
                    VGA.GetFrameBuffer()->DrawShape(s);
                }
            }
        }
        break;
        case ConvFrame::CONV_WIDE:
        {
            for (size_t i = 0; i < this->current_frame->participants.size(); i++) {
                CharFigure *participant = this->current_frame->participants[i];

                if (i == 0) {
                    ByteStream paletteReader;
                    paletteReader.Set(convPals.GetEntry(participant->paletteID)->data);
                    this->palette.ReadPatch(&paletteReader);
                    VGA.SetPalette(&this->palette);
                }
                
                RLEShape *s = participant->appearances->GetShape(0);
                Point2D position = {participant->x, participant->y};
                s->SetPosition(&position);
                VGA.GetFrameBuffer()->DrawShape(s);
                if (participant->appearances->GetNumImages() > 1) {
                    s = participant->appearances->GetShape(1);
                    s->SetPosition(&position);
                    VGA.GetFrameBuffer()->DrawShape(s);
                }
                position.x += s->GetWidth();
            }

            for (size_t i = 0; i < CONV_TOP_BAR_HEIGHT; i++)
                VGA.GetFrameBuffer()->FillLineColor(i, 0x00);

            for (size_t i = 0; i < CONV_BOTTOM_BAR_HEIGHT; i++)
                VGA.GetFrameBuffer()->FillLineColor(199 - i, 0x00);
        }
        break;
        case  ConvFrame::CONV_WINGMAN_CHOICE:
        {
            Point2D position = {0, 0};
            for (size_t i = 0; i < this->current_frame->participants.size(); i++) {
                CharFigure *participant = this->current_frame->participants[i];
                ByteStream paletteReader;
                paletteReader.Set(convPals.GetEntry(participant->paletteID)->data);
                this->palette.ReadPatch(&paletteReader);
                VGA.SetPalette(&this->palette);
                RLEShape *s = participant->appearances->GetShape(0);
                s->SetPosition(&position);
                VGA.GetFrameBuffer()->DrawShape(s);
                if (participant->appearances->GetNumImages() > 1) {
                    s = participant->appearances->GetShape(1);
                    s->SetPosition(&position);
                    VGA.GetFrameBuffer()->DrawShape(s);
                }
                position.x += s->GetWidth();
            }
            CheckZones();
            Mouse.Draw();
        }
        break;
        case ConvFrame::CONV_CONTRACT_CHOICE:
        {
            ByteStream paletteReader;
            paletteReader.Set(convPals.GetEntry(this->current_frame->facePaletteID)->data);
            this->palette.ReadPatch(&paletteReader);
            VGA.SetPalette(&this->palette);
            int32_t pos = 0;

            if (this->current_frame->mode == ConvFrame::CONV_CLOSEUP) {
                if (this->current_frame->facePosition == ConvFrame::FACE_LEFT)
                    pos = -30;

                if (this->current_frame->facePosition == ConvFrame::FACE_RIGHT)
                    pos = 30;
            }

            if (this->current_frame->face != NULL) {
                RLEShape *s = nullptr;
                if (faces_shape.find(this->current_frame->face_expression) != faces_shape.end()) {
                    for (auto shape : faces_shape[this->current_frame->face_expression]) {
                        s = this->current_frame->face->appearances->GetShape(shape);
                        s->SetPositionX(pos);
                        VGA.GetFrameBuffer()->DrawShape(s);
                    }
                }
                s = this->current_frame->face->appearances->GetShape(54);
                s->SetPositionX(pos);
                VGA.GetFrameBuffer()->DrawShape(s);
            }
            CheckZones();
            Mouse.Draw();
        }   
        break;
    }
    if (this->current_frame->sound_file_name != nullptr && this->current_frame->sound_file_name->length()>0 && Mixer.IsSoundPlaying() == false) {
        this->current_frame->SetExpired(true);
    }
    this->DrawText();
    
    VGA.VSync();
}
