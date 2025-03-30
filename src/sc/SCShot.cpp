//
//  SCShot.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 23/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//


#include "precomp.h"

SCShot::SCShot() { this->fps = SDL_GetTicks() / 100; }

SCShot::~SCShot() {}

/**
 * Checks for keyboard events and updates the game state accordingly.
 *
 * This function peeks at the SDL event queue to check for keyboard events.
 * If a key is pressed, it updates the game state based on the key pressed.
 * The supported keys are escape, space, and return.
 *
 * @return None
 */
void SCShot::CheckKeyboard(void) {
    SDL_Event mouseEvents[5];
    int numMouseEvents = SDL_PeepEvents(mouseEvents, 5, SDL_PEEKEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP);
    for (int i = 0; i < numMouseEvents; i++) {
        SDL_Event *event = &mouseEvents[i];

        switch (event->type) {
        case SDL_MOUSEBUTTONUP:
            Game.StopTopActivity();
            break;
        default:
            break;
        }
    }
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event *event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        default:
            break;
        }
    }
}

/**
 * Initializes the SCShot object by loading the necessary IFF files and initializing the optionParser and
 * gameFlowParser. It also initializes the optShps and optPals objects by loading the OPTSHPS.PAK and OPTPALS.PAK files
 * respectively. Finally, it sets the efect pointer to nullptr and calls the createMiss() function.
 *
 * @throws None
 */
void SCShot::Init() {

    TreEntry *optionIFF =
        Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTIONS.IFF");
    this->optionParser.InitFromRam(optionIFF->data, optionIFF->size);
    TreEntry *optShapEntry =
        Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
    this->optShps.InitFromRAM("OPTSHPS.PAK", optShapEntry->data, optShapEntry->size);
    TreEntry *optPalettesEntry =
        Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
    this->optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
}

/**
 * Sets the shot ID of the SCShot object and updates the layers and palette accordingly.
 *
 * @param shotid The shot ID to set.
 *
 * @throws None
 */
void SCShot::SetShotId(uint8_t shotid) {
    for (auto s : this->optionParser.estb[shotid]->images) {
        shotBackground *bg = new shotBackground();
        bg->img = this->getShape(s->OptshapeID);
        this->layers.push_back(bg);
    }
    uint8_t paltID = this->optionParser.estb[shotid]->palettes[0]->ID;
    this->rawPalette = this->optPals.GetEntry(paltID)->data;
    for (auto p : this->optionParser.estb[shotid]->palettes) {
        this->palettes.push_back(this->optPals.GetEntry(p->ID)->data);
    }
}

/**
 * Retrieves an RSImageSet object for the specified shape ID.
 *
 * @param shpid The ID of the shape to retrieve.
 *
 * @return A pointer to the RSImageSet object containing the shape data.
 *
 * @throws None.
 */
RSImageSet *SCShot::getShape(uint8_t shpid) {
    PakEntry *shapeEntry = this->optShps.GetEntry(shpid);
    PakArchive subPAK;

    subPAK.InitFromRAM("", shapeEntry->data, shapeEntry->size);
    RSImageSet *img = new RSImageSet();
    if (!subPAK.IsReady()) {
        img->InitFromPakEntry(this->optShps.GetEntry(shpid));
    } else {
        img->InitFromSubPakEntry(&subPAK);
    }
    return (img);
}

/**
 * Runs a single frame of the game, updating the game state and rendering the graphics.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 */
void SCShot::RunFrame(void) {
    CheckKeyboard();
    VGA.Activate();
    VGA.GetFrameBuffer()->FillWithColor(0);
    ByteStream paletteReader;
    int fpsupdate = 0;
    fpsupdate = (SDL_GetTicks() / 10) - this->fps > 12;
    if (fpsupdate) {
        this->fps = (SDL_GetTicks() / 10);
    }

    bool ended = false;
    int cpt = 0;
    for (auto layer : this->layers) {
        if (cpt < this->palettes.size()) {
            paletteReader.Set((this->palettes[cpt]));
            this->palette.ReadPatch(&paletteReader);
            VGA.SetPalette(&this->palette);
        }

        if (layer->img->palettes.size() >= 1) {
            layer->img->palettes[0]->copyFrom(&this->palette);
            VGA.SetPalette(layer->img->palettes[0]->GetColorPalette());
        } else {
            VGA.SetPalette(&this->palette);
        }
        VGA.GetFrameBuffer()->DrawShape(layer->img->GetShape(layer->img->sequence[layer->frameCounter]));
        if (layer->img->sequence.size() > 1) {
            layer->frameCounter = layer->frameCounter + fpsupdate;
            if (layer->frameCounter >= layer->img->sequence.size()) {
                ended = true;
            }
        }
        cpt++;
    }
    for (size_t i = 0; i < CONV_TOP_BAR_HEIGHT; i++)
        VGA.GetFrameBuffer()->FillLineColor(i, 0x00);

    for (size_t i = 0; i < CONV_BOTTOM_BAR_HEIGHT; i++)
        VGA.GetFrameBuffer()->FillLineColor(199 - i, 0x00);
    Mouse.Draw();

    VGA.VSync();
    if (ended) {
        Game.StopTopActivity();
    }
}

void EndMissionScene::Init() {
    // opt shp id 143
    // opt shp id 141
    SCShot::Init();
    this->layers.clear();
    shotBackground *tmpbg = new shotBackground();
    tmpbg->img = this->getShape(143);
    this->layers.push_back(tmpbg);
    tmpbg = new shotBackground();
    tmpbg->img = this->getShape(141);
    this->layers.push_back(tmpbg);
    this->rawPalette = this->optPals.GetEntry(20)->data;
}

void EndMissionScene::RunFrame() {
    CheckKeyboard();
    int fpsupdate = 0;
    fpsupdate = (SDL_GetTicks() / 10) - this->fps > 12;
    if (fpsupdate) {
        this->fps = (SDL_GetTicks() / 10);
    }
    Mixer.SwitchBank(0);
    Mixer.PlayMusic(27, 1);
    ByteStream paletteReader;
    paletteReader.Set((this->rawPalette));
    this->palette.ReadPatch(&paletteReader);
    VGA.Activate();
    VGA.GetFrameBuffer()->FillWithColor(0);
    VGA.SetPalette(&this->palette);
    shotBackground *layer = this->layers[this->part];
    VGA.GetFrameBuffer()->DrawShape(layer->img->GetShape(layer->img->sequence[0]));
    VGA.GetFrameBuffer()->DrawShape(layer->img->GetShape(layer->img->sequence[layer->frameCounter]));
    if (layer->img->sequence.size() > 1) {
        layer->frameCounter = (uint8_t)(layer->frameCounter + fpsupdate) % layer->img->sequence.size();
    }
    Mouse.Draw();
    VGA.VSync();
}

void EndMissionScene::CheckKeyboard(void) {
    SDL_Event mouseEvents[5];
    int numMouseEvents = SDL_PeepEvents(mouseEvents, 5, SDL_PEEKEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP);
    for (int i = 0; i < numMouseEvents; i++) {
        SDL_Event *event = &mouseEvents[i];

        switch (event->type) {
        case SDL_MOUSEBUTTONUP:
            if (this->part == 0) {
                this->part = 1;
                this->rawPalette = this->optPals.GetEntry(19)->data;
            } else {
                Game.StopTopActivity();
            }
            break;
        default:
            break;
        }
    }
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event *event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        default:
            break;
        }
    }
}
void MapShot::Init() {
    SCShot::Init();
    x_max = 960;
    y_max = 600;
    this->frameBuffer = new FrameBuffer(x_max, y_max);
    this->frameBuffer->FillWithColor(0);
    this->frameBufferA = new FrameBuffer(x_max, y_max);
    this->frameBufferA->FillWithColor(255);
    this->layers.clear();
    this->rawPalette = this->optPals.GetEntry(23)->data;
    std::map<uint8_t, std::tuple<uint16_t, uint16_t>> map = {
        {128, {177, 37}},
        {129, {177, 237}},
        {130, {497, 37}},
        {131, {497, 237}},
        {132, {0, 200}},
        {133, {177, 0}},
        {134, {0, 0}}
     };
    std::vector<uint8_t> shpids_to_paint = {134, 132, 133, 128, 129, 130, 131};

    for (auto i : shpids_to_paint) {
        shotBackground *tmpbg = new shotBackground();
        tmpbg->img = this->getShape(i);
        this->layers.push_back(tmpbg);
        uint8_t *tmp_fb = new uint8_t[320 * 200];
        size_t read = 0;
        tmpbg->img->GetShape(tmpbg->img->sequence[0])->Expand(tmp_fb, &read);
        this->mapics.push_back(tmp_fb);
        this->frameBuffer->blit(tmp_fb, std::get<0>(map[i]), std::get<1>(map[i]), 320, 200);
    }
    
    this->font = FontManager.GetFont("..\\..\\DATA\\FONTS\\OPTFONT.SHP");
}
void MapShot::SetPoints(std::vector<MAP_POINT *> *points) { 
    this->points = points;
    point_counter = 0;
    next_point_counter = 1;
    point_x_move =
        (((*this->points)[next_point_counter]->x) - ((*this->points)[point_counter]->x)) / 100.0f;
    point_y_move =
        (((*this->points)[next_point_counter]->y) - ((*this->points)[point_counter]->y)) / 100.0f;
    current_x = (float) (*this->points)[point_counter]->x;
    current_y = (float) (*this->points)[point_counter]->y;
}

void MapShot::RunFrame(void) {
    CheckKeyboard();
    ByteStream paletteReader;
    paletteReader.Set((this->rawPalette));
    this->palette.ReadPatch(&paletteReader);
    VGA.Activate();
    VGA.GetFrameBuffer()->FillWithColor(255);
    VGA.SetPalette(&this->palette);
    static int nb_etapes = 100;
    this->fp_counter++;

    bool ended = false;
    
    if (nb_etapes < 0) {
        point_counter++;
        nb_etapes = 100;
        if (point_counter >= this->points->size()) {
            point_counter = 0;
        }
        next_point_counter = point_counter + 1;
        if (next_point_counter >= this->points->size()) {
            next_point_counter = 0;
            ended = true;
        }
        current_x = (float) (*this->points)[point_counter]->x;
        current_y = (float) (*this->points)[point_counter]->y;
        point_x_move = (((*this->points)[next_point_counter]->x) - ((*this->points)[point_counter]->x)) / 100.0f;
        point_y_move = (((*this->points)[next_point_counter]->y) - ((*this->points)[point_counter]->y)) / 100.0f;
    }
    if (fp_counter % 10 == 0) {
        nb_etapes--;
        current_x += point_x_move;
        current_y += point_y_move;
    }

    this->x = (int) current_x - 160;
    this->y = (int) current_y - 100;
    
    frameBufferA->framebuffer[(int)current_y * x_max + (int)current_x] = 246;
    VGA.GetFrameBuffer()->blitLargeBuffer(frameBuffer->framebuffer, x_max, y_max, this->x, this->y, 0, 0, 320, 200);
    float rx = (this->x / (x_max / 320.0f));
    float ry = (this->y / (y_max / 200.0f)) + 100;
    
    VGA.GetFrameBuffer()->blitLargeBuffer(frameBufferA->framebuffer, x_max, y_max, this->x, this->y, 0, 0, 320, 200);
    Mouse.Draw();
    for (auto p: *this->points) {
        VGA.GetFrameBuffer()->plot_pixel((int)p->x, (int)p->y, 0);
        VGA.GetFrameBuffer()->PrintText_SM(
            this->font,
            new Point2D{p->x-this->x, p->y-this->y},
            (char*)p->label.c_str(),
            90,
            0,
            (uint32_t) p->label.size(),
            1,
            this->font->GetShapeForChar('A')->GetWidth(),
            false, false
        );
    }
    VGA.VSync();
    if (ended) {
        Game.StopTopActivity();
    }
}
void MapShot::CheckKeyboard(void) {
    SDL_Event mouseEvents[5];
    int numMouseEvents = SDL_PeepEvents(mouseEvents, 5, SDL_PEEKEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP);
    for (int i = 0; i < numMouseEvents; i++) {
        SDL_Event *event = &mouseEvents[i];

        switch (event->type) {
        case SDL_MOUSEBUTTONUP:
            Game.StopTopActivity();
            break;
        default:
            break;
        }
    }
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event *event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game.StopTopActivity();
            break;
        }
        case SDLK_c:
            this->color = (this->color + 1) % 255;
            break;
        default:
            break;
        }
    }
}