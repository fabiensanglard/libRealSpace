//
//  SCNavMap.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 31/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include "precomp.h"

SCNavMap::SCNavMap(){
    this->navMap = nullptr;
}
SCNavMap::~SCNavMap(){

}
/**
 * Checks for keyboard events and updates the game state accordingly.
 *
 * This function peeks at the SDL event queue to check for keyboard events.
 * If a key is pressed, it updates the game state based on the key pressed.
 * The supported keys are escape, space, and return.
 *
 * @return None
 */
void SCNavMap::CheckKeyboard(void) {
    //Keyboard
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event* event = &keybEvents[i];

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
 * Initializes the SCNavMap object.
 *
 * This function reads in the PALETTE.IFF file and sets the object's color palette.
 * It also reads in the NAMAP.IFF file and sets the object's navMap property.
 *
 * @return None
 */
void SCNavMap::Init(){
    IffLexer lexer;
    lexer.InitFromFile("PALETTE.IFF");
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();

    TreEntry *nav_map = Assets.tres[AssetManager::TRE_MISSIONS]->GetEntryByName("..\\..\\DATA\\COCKPITS\\NAVMAP.IFF");
    this->navMap = new RSNavMap();
    this->navMap->InitFromRam(nav_map->data, nav_map->size);
}
/**
 * Sets the name of the current mission.
 *
 * This function takes a C-style string as its argument and sets the object's
 * name to that string.
 *
 * @param name the name of the current mission
 */
void SCNavMap::SetName(char *name) {
    this->name = new std::string(name);
}
/**
 * SCNavMap::RunFrame
 *
 * This function runs the frame of the navigation map.
 *
 * It clears the screen, sets the palette, draws the background and the shape
 * corresponding to the current mission, and draws the mouse.
 *
 * @return None
 */
void SCNavMap::RunFrame(void) {
    this->CheckKeyboard();
    VGA.Activate();
    VGA.Clear();
    
    VGA.SetPalette(&this->palette);
    
    //Draw static
    VGA.DrawShape(this->navMap->background);
    if (this->navMap->maps.count(*this->name)>0) {
        VGA.DrawShape(this->navMap->maps[*this->name]);
    }
    Mouse.Draw();
    VGA.VSync();
}