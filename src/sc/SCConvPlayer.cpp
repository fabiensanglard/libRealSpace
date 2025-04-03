//
//  SCConvPlayer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

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
    { 22,     std::vector<uint8_t>{1, 23, 2, 35}},
    { 23,     std::vector<uint8_t>{1, 24, 2, 35}},
    { 24,     std::vector<uint8_t>{1, 25, 2, 35}},
    { 25,     std::vector<uint8_t>{1, 26, 2, 35}},
    { 31,         std::vector<uint8_t>{1, 2, 35}},
    {255,         std::vector<uint8_t>{1, 2, 35}},
};

void SCConvPlayer::Focus(void) { IActivity::Focus(); }

void SCConvPlayer::clicked(void *none, uint8_t id) {
    printf("clicked on %d\n", id);
    if (id == 1) {
        GameState.mission_accepted = true;
        conv.MoveForward(yesOffset);
    } else if (id == 2) {
        GameState.mission_accepted = false;
        conv.MoveForward(noOffset);
    }
    currentFrame.SetExpired(true);
}
void SCConvPlayer::selectWingMan(void *none, uint8_t id) {
    printf("clicked on %d\n", id);
    currentFrame.SetExpired(true);
}
/**
 * @brief Read the next frame of the conversation from the data stream.
 *
 * If the end of the conversation is reached, stop the activity.
 *
 * The function reads the next byte from the stream and uses it to determine the type of frame to read.
 * Depending on the type of frame, it reads the appropriate data from the stream and stores it in currentFrame.
 * If the type of frame is unknown, it stops the activity and prints an error message.
 */
void SCConvPlayer::ReadNextFrame(void) {

    if (conv.GetPosition() == end) {
        Stop();
        return;
    }

    currentFrame.creationTime  = SDL_GetTicks();
    currentFrame.text          = nullptr;
    currentFrame.facePaletteID = 0;
    // currentFrame.face = nullptr;

    uint8_t type = conv.ReadByte();

    switch (type) {
    case GROUP_SHOT: // Group plan
    {
        char *location = (char *)conv.GetPosition();

        ConvBackGround *bg = ConvAssets.GetBackGround(location);

        currentFrame.mode = ConvFrame::CONV_WIDE;
        currentFrame.participants.clear();
        currentFrame.bgLayers   = &bg->layers;
        currentFrame.bgPalettes = &bg->palettes;

        printf("ConvID: %d WIDEPLAN : LOCATION: '%s'\n", this->conversationID, location);
        conv.MoveForward(8);
        if (conv.PeekByte() == GROUP_SHOT_ADD_CHARCTER) {
            conv.MoveForward(1);
            ReadNextFrame();
        } else if (conv.PeekByte() == SHOW_TEXT) {
            conv.MoveForward(1);
            ReadNextFrame();
        } else {
            conv.MoveForward(1);
        }

        break;
    }
    case CLOSEUP: // Person talking
    {
        char *speakerName            = (char *)conv.GetPosition();
        currentFrame.face_expression = (uint8_t)*(conv.GetPosition() + 0x09);
        char *setName                = (char *)conv.GetPosition() + 0xA;

        char *sentence = (char *)conv.GetPosition() + 0x17;

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

        uint8_t pos               = *(conv.GetPosition() + 0x13);
        currentFrame.facePosition = static_cast<ConvFrame::FacePos>(pos);
        currentFrame.text         = (char *)text->c_str();
        currentFrame.mode         = ConvFrame::CONV_CLOSEUP;
        currentFrame.face         = ConvAssets.GetCharFace(speakerName);
        ConvBackGround *bg        = ConvAssets.GetBackGround(setName);
        currentFrame.bgLayers     = &bg->layers;
        currentFrame.bgPalettes   = &bg->palettes;

        conv.MoveForward(0x17 + strlen((char *)sentence) + 1);
        uint8_t color              = conv.ReadByte(); // Color ?
        currentFrame.textColor     = color;
        const char *pszExt         = "normal";
        currentFrame.facePaletteID = ConvAssets.GetFacePaletteID(const_cast<char *>(pszExt));

        printf(
            "ConvID: %d CLOSEUP: WHO: '%8s' WHERE: '%8s'     WHAT: '%s' (%2X) pos %2X  face expression: '%d'\n",
            this->conversationID, speakerName, setName, sentence, color, pos, currentFrame.face_expression
        );
        break;
    }
    case CLOSEUP_CONTINUATION: // Same person keep talking
    {
        char *sentence = (char *)conv.GetPosition();

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
        currentFrame.text = (char *)text->c_str();

        conv.MoveForward(strlen((char *)sentence) + 1);
        printf(
            "ConvID: %d MORETEX:                                       WHAT: '%s'\n", this->conversationID, sentence
        );
        break;
    }
    case YESNOCHOICE_BRANCH1: // Choice Offsets are question
    {
        currentFrame.mode = ConvFrame::CONV_CONTRACT_CHOICE;
        printf("ConvID: %d CHOICE YES/NO : %d\n", this->conversationID, type);
        // Looks like first byte is the offset to skip if the answer is no.
        this->noOffset  = conv.ReadByte();
        this->yesOffset = conv.ReadByte();
        printf("Offsets: %d %d\n", this->noOffset, this->yesOffset);
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
        zone->onclick = std::bind(&SCConvPlayer::clicked, this, std::placeholders::_1, std::placeholders::_2);
        this->zones.push_back(zone);

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
        zone->onclick = std::bind(&SCConvPlayer::clicked, this, std::placeholders::_1, std::placeholders::_2);
        this->zones.push_back(zone);
        break;
    }
    case YESNOCHOICE_BRANCH2: // Choice offset after first branch
    {

        // currentFrame.mode = ConvFrame::CONV_CONTRACT_CHOICE;
        printf("ConvID: %d CHOICE YES/NO : %d\n", this->conversationID, type);
        // Looks like first byte is the offset to skip if the answer is no.
        uint8_t t = conv.ReadByte();
        uint8_t b = conv.ReadByte();
        if (GameState.mission_accepted) {
            conv.MoveForward(t);
        }
        break;
    }
    case GROUP_SHOT_ADD_CHARCTER: // Add person to GROUP
    {

        char *participantName   = (char *)conv.GetPosition();
        CharFigure *participant = ConvAssets.GetFigure(participantName);
        currentFrame.participants.push_back(participant);

        printf("ConvID: %d WIDEPLAN ADD PARTICIPANT: '%s'\n", this->conversationID, conv.GetPosition());
        conv.MoveForward(12);
        if (conv.PeekByte() == GROUP_SHOT_ADD_CHARCTER) {
            conv.MoveForward(1);
            ReadNextFrame();
        } else {
            conv.MoveForward(1);
        }
        break;
    }
    case GROUP_SHOT_CHARCTR_TALK: // Make group character talk
    {

        char *who = (char *)conv.GetPosition();
        conv.MoveForward(0xE);
        char *sentence    = (char *)conv.GetPosition();
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
        currentFrame.text = (char *)text->c_str();
        conv.MoveForward(strlen(sentence) + 1);
        printf("ConvID: %d WIDEPLAN PARTICIPANT TALKING: who: '%s' WHAT '%s'\n", this->conversationID, who, sentence);
        CharFigure *participant = ConvAssets.GetFigure(who);
        currentFrame.participants.push_back(participant);
        break;
    }
    case SHOW_TEXT: // Show text
    {
        int8_t color      = conv.ReadByte();
        char *sentence    = (char *)conv.GetPosition();
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
        currentFrame.mode = ConvFrame::CONV_WIDE;
        currentFrame.text = (char *)text->c_str();
        ;
        currentFrame.textColor = color;
        currentFrame.face      = nullptr;
        printf("ConvID: %d Show Text: '%s' \n", this->conversationID, sentence);
        conv.MoveForward(strlen(sentence) + 1);

        break;
    }
    case 0xE: {
        uint8_t unkn  = conv.ReadByte();
        uint8_t unkn1 = conv.ReadByte();
        printf("ConvID: %d Unknown usage Flag 0xE: (0x%2X 0x%2X) \n", this->conversationID, unkn, unkn1);
        ReadNextFrame();
        break;
    }
    case CHOOSE_WINGMAN: // Wingman selection trigger
    {
        ConvBackGround *bg = ConvAssets.GetBackGround("s_wing");

        currentFrame.participants.clear();
        currentFrame.bgLayers   = &bg->layers;
        currentFrame.bgPalettes = &bg->palettes;

        currentFrame.participants.clear();
        currentFrame.mode = ConvFrame::CONV_WINGMAN_CHOICE;
        printf("ConvID: %d Open pilot selection screen with current BG.\n", this->conversationID);
        CharFigure *entry = ConvAssets.GetFigure("red1");
        currentFrame.participants.push_back(entry);
        std::vector<std::string> airwing = {"billy2", "gwen2", "lyle2", "miguel2", "tex3"};
        std::vector<std::string> airwing_names = {"Lyle", "Miguel", "Gwen", "Billy", "Tex"};
        std::vector<std::vector<Point2D>> zonespos = {
            { { 0, 80 }, { 60, 80 }, { 60, 200 }, { 0, 200 } },
            { { 64, 80 }, { 110, 80 }, { 110, 200 }, { 64, 200 } },
            { { 120, 80 }, { 170, 80 }, { 170, 200 }, { 120, 200 } },
            { { 175, 80 }, { 218, 80 }, { 218, 200 }, { 175, 200 } },
            { { 228, 80 }, { 268, 80 }, { 268, 200 }, { 228, 200 } }
        };
        this->zones.clear();
        this->zones.shrink_to_fit();
        int i = 0;
        for (auto pilot : airwing) {
            CharFigure *participant = ConvAssets.GetFigure((char *)pilot.c_str());
            currentFrame.participants.push_back(participant);
            SCZone *zone = new SCZone();
            zone->label  = new std::string("Select "+airwing_names[i]);
            zone->quad   = new std::vector<Point2D *>();
            for (auto zp: zonespos[i]) {
                Point2D *p = new Point2D{zp.x, zp.y};
                zone->quad->push_back(p);
            }
            zone->id = i;
            zone->active = true;
            zone->onclick = std::bind(&SCConvPlayer::selectWingMan, this, std::placeholders::_1, std::placeholders::_2);
            this->zones.push_back(zone);
            i++;
        }

        break;
    }
    default:
        printf("ConvID: %d Unknown opcode: %X.\n", this->conversationID, type);
        Stop();
        return;
        break;
    }

    this->currentFrame.SetExpired(false);
}

void SCConvPlayer::SetArchive(PakEntry *convPakEntry) {

    if (convPakEntry->size == 0) {
        Game.Log("Conversation entry is empty: Unable to load it.\n");
        Stop();
        return;
    }

    this->size = convPakEntry->size;

    this->conv.Set(convPakEntry->data);
    end = convPakEntry->data + convPakEntry->size;

    // Read a frame so we are ready to display it.
    // ReadNextFrame();
    this->currentFrame.SetExpired(true);

    initialized = true;
}

void SCConvPlayer::SetID(int32_t id) {

    this->conversationID = id;

    TreEntry *convEntry = Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONV.PAK");

    PakArchive convPak;
    convPak.InitFromRAM("CONV.PAK", convEntry->data, convEntry->size);
    // convPak.List(stdout);

    if (convPak.GetNumEntries() <= id) {
        Stop();
        Game.Log("Cannot load conversation id (max convID is %lu).", convPak.GetNumEntries() - 1);
        return;
    }

    SetArchive(convPak.GetEntry(id));
}

void SCConvPlayer::Init() {
    VGAPalette *rendererPalette = VGA.GetPalette();
    this->palette               = *rendererPalette;

    currentFrame.font = FontManager.GetFont("..\\..\\DATA\\FONTS\\CONVFONT.SHP");
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
    if (currentFrame.mode != ConvFrame::CONV_CONTRACT_CHOICE && currentFrame.mode != ConvFrame::CONV_WINGMAN_CHOICE) {
        // A frame expires either after a player press a key, click or 6 seconds elapse.
        // Mouse
        SDL_Event mouseEvents[5];
        int numMouseEvents = SDL_PeepEvents(mouseEvents, 5, SDL_PEEKEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP);
        for (int i = 0; i < numMouseEvents; i++) {
            SDL_Event *event = &mouseEvents[i];

            switch (event->type) {
            case SDL_MOUSEBUTTONUP:
                this->currentFrame.SetExpired(true);
                break;
            default:
                break;
            }
        }

        // Keyboard
        SDL_Event keybEvents[5];
        int numKeybEvents = SDL_PeepEvents(keybEvents, 5, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
        for (int i = 0; i < numKeybEvents; i++) {
            SDL_Event *event = &keybEvents[i];
            switch (event->key.keysym.sym) {
            case SDLK_SPACE:
                this->currentFrame.SetExpired(true);
                break;
            case SDLK_ESCAPE:
                Game.StopTopActivity();
                break;
            default:
                this->currentFrame.SetExpired(true);
                break;
            }
        }

    
        int32_t currentTime = SDL_GetTicks();
        if (currentTime - currentFrame.creationTime > 5000)
            this->currentFrame.SetExpired(true);
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

    if (currentFrame.text == NULL)
        return;

    size_t textSize    = strlen(currentFrame.text);
    const char *cursor = currentFrame.text;
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

                if (*wordSearch == ' ')
                    pixelAvailable -= CONV_SPACE_SIZE;
                else
                    pixelAvailable -=
                        currentFrame.font->GetShapeForChar(*wordSearch)->GetWidth() + CONV_INTERLETTER_SPACE;

                wordSearch++;
            }

            if (pixelAvailable > 0)
                lastGoodPos = currentFrame.text + strlen(currentFrame.text);
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
            currentFrame.font, &coo, currentFrame.text, static_cast<uint32_t>(currentFrame.textColor),
            static_cast<uint32_t>(cursor - currentFrame.text), static_cast<uint32_t>(lastGoodPos - cursor),
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
void SCConvPlayer::RunFrame(void) {

    if (!initialized) {
        Stop();
        Game.Log("Conv ID %d was not initialized: Stopping.\n", this->conversationID);
        return;
    }

    // If frame needs to be update
    if (currentFrame.mode != ConvFrame::CONV_CONTRACT_CHOICE && currentFrame.mode != ConvFrame::CONV_WINGMAN_CHOICE) {
        CheckFrameExpired();
    }

    if (currentFrame.IsExpired()) {

        ReadNextFrame();
    }

    CheckButtons();

    VGA.Activate();
    VGA.GetFrameBuffer()->FillWithColor(255);
    TreEntry *convPalettesEntry =
        Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\CONVPALS.PAK");
    PakArchive convPals;
    convPals.InitFromRAM("CONVPALS.PAK", convPalettesEntry->data, convPalettesEntry->size);

    // Update the palette for the current background
    for (size_t i = 0; i < currentFrame.bgLayers->size(); i++) {
        ByteStream paletteReader;
        paletteReader.Set((*currentFrame.bgPalettes)[i]);
        this->palette.ReadPatch(&paletteReader);
        VGA.SetPalette(&this->palette);
    }

    // Draw static
    for (size_t i = 0; i < currentFrame.bgLayers->size(); i++) {
        RLEShape *shape = (*currentFrame.bgLayers)[i];
        VGA.GetFrameBuffer()->DrawShape(shape);
    }

    if (currentFrame.mode == ConvFrame::CONV_CLOSEUP) {

        for (size_t i = 0; i < CONV_TOP_BAR_HEIGHT + 1; i++)
            VGA.GetFrameBuffer()->FillLineColor(i, 0x00);

        for (size_t i = 0; i < CONV_BOTTOM_BAR_HEIGHT; i++)
            VGA.GetFrameBuffer()->FillLineColor(199 - i, 0x00);
    }

    //
    if (currentFrame.mode == ConvFrame::CONV_CLOSEUP || currentFrame.mode == ConvFrame::CONV_CONTRACT_CHOICE) {

        ByteStream paletteReader;
        paletteReader.Set(convPals.GetEntry(currentFrame.facePaletteID)->data);
        this->palette.ReadPatch(&paletteReader);

        int32_t pos = 0;

        if (currentFrame.mode == ConvFrame::CONV_CLOSEUP) {
            if (currentFrame.facePosition == ConvFrame::FACE_LEFT)
                pos = -30;

            if (currentFrame.facePosition == ConvFrame::FACE_RIGHT)
                pos = 30;
        }

        if (currentFrame.face != NULL) {
            RLEShape *s = nullptr;
            if (faces_shape.find(currentFrame.face_expression) != faces_shape.end()) {
                for (auto shape : faces_shape[currentFrame.face_expression]) {
                    s = currentFrame.face->appearances->GetShape(shape);
                    s->SetPositionX(pos);
                    VGA.GetFrameBuffer()->DrawShape(s);
                }
            }
            for (size_t i = 03; i < 11 && currentFrame.mode == ConvFrame::CONV_CLOSEUP &&
                                currentFrame.text != nullptr && currentFrame.text[0] != '\0';
                i++) {
                RLEShape *s = currentFrame.face->appearances->GetShape(3 + (SDL_GetTicks() / 100) % 10);
                s->SetPositionX(pos);
                VGA.GetFrameBuffer()->DrawShape(s);
            }

            if (currentFrame.mode == ConvFrame::CONV_CONTRACT_CHOICE) {
                RLEShape *s = currentFrame.face->appearances->GetShape(54);
                s->SetPositionX(pos);
                VGA.GetFrameBuffer()->DrawShape(s);
            }
        }
    }

    if (currentFrame.mode == ConvFrame::CONV_WIDE) {
        Point2D position = {0, CONV_TOP_BAR_HEIGHT + 1};
        for (size_t i = 0; i < currentFrame.participants.size(); i++) {
            CharFigure *participant = currentFrame.participants[i];
            ByteStream paletteReader;
            paletteReader.Set(convPals.GetEntry(participant->paletteID)->data);
            this->palette.ReadPatch(&paletteReader);
            RLEShape *s = participant->appearances->GetShape(0);
            s->SetPosition(&position);
            VGA.GetFrameBuffer()->DrawShape(s);
            position.x += s->GetWidth();
        }

        for (size_t i = 0; i < CONV_TOP_BAR_HEIGHT; i++)
            VGA.GetFrameBuffer()->FillLineColor(i, 0x00);

        for (size_t i = 0; i < CONV_BOTTOM_BAR_HEIGHT; i++)
            VGA.GetFrameBuffer()->FillLineColor(199 - i, 0x00);
    }
    if (currentFrame.mode == ConvFrame::CONV_WINGMAN_CHOICE) {
        Point2D position = {0, 0};
        for (size_t i = 0; i < currentFrame.participants.size(); i++) {
            CharFigure *participant = currentFrame.participants[i];
            ByteStream paletteReader;
            paletteReader.Set(convPals.GetEntry(participant->paletteID)->data);
            this->palette.ReadPatch(&paletteReader);
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
        for (auto zone : zones) {
            zone->drawQuad();
        }
        CheckZones();
        Mouse.Draw();
    }

    if (currentFrame.mode == ConvFrame::CONV_CONTRACT_CHOICE) {
        CheckZones();
        Mouse.Draw();
    }
    // Draw text
    DrawText();

    DrawButtons();

    VGA.VSync();
}
