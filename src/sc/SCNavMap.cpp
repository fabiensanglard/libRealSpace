#include "precomp.h"

SCNavMap::SCNavMap(){
    this->navMap = nullptr;
}
SCNavMap::~SCNavMap(){

}
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
void SCNavMap::SetName(char *name) {
    this->name = new std::string(name);
}
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