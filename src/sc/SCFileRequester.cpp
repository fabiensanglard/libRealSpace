#include "precomp.h"
#include "SCFileRequester.h"

SCFileRequester::SCFileRequester() {
    RSImageSet *uiImageSet = new RSImageSet();
    TreEntry *tre = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\SAVELOAD.SHP");
    PakArchive *pak = new PakArchive();
    pak->InitFromRAM("SAVELOAD.SHP", tre->data, tre->size);
    for (int i = 0; i < pak->GetNumEntries(); i++) {
        RLEShape *shape = new RLEShape();
        shape->Init(pak->GetEntry(i)->data+8, pak->GetEntry(i)->size);
        uiImageSet->shapes.push_back(shape);
    }
    this->uiImageSet = uiImageSet;
}
SCFileRequester::~SCFileRequester() {
    delete this->uiImageSet;
}
void SCFileRequester::draw(FrameBuffer *fb) {
    // 182x131
    FrameBuffer *fb2 = new FrameBuffer(182, 131);
    fb2->DrawShape(this->uiImageSet->GetShape(0));
    
    fb->blit(fb2->framebuffer, (320-182)/2, (200-131)/2, fb2->width, fb2->height);
}
void SCFileRequester::checkevents() {
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event *event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            this->opened = false;
            break;
        }
        default:
            break;
        }
    }
}