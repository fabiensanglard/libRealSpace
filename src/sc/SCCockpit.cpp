//
//  SCCockpit.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 02/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include "precomp.h"

SCCockpit::SCCockpit() {

}
SCCockpit::~SCCockpit(){

}
/**
 * SCCockpit::Init
 *
 * Initialize the cockpit object from the standard SC cockpit assets.
 *
 * This function reads the palette from the standard SC palette IFF file,
 * initializes an RSPalette object from it, and assigns the result to the
 * SCCockpit::palette member variable.
 *
 * It also initializes the cockpit object from the F16 cockpit IFF file
 * and assigns the result to the SCCockpit::cockpit member variable.
 */
void SCCockpit::Init( ) {
    IffLexer lexer;
    lexer.InitFromFile("PALETTE.IFF");
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();
    cockpit = new RSCockpit();
    TreEntry *cockpit_def = Assets.tres[AssetManager::TRE_OBJECTS]->GetEntryByName("..\\..\\DATA\\OBJECTS\\F16-CKPT.IFF");
    cockpit->InitFromRam(cockpit_def->data, cockpit_def->size);

    TreEntry *hud_def = Assets.tres[AssetManager::TRE_OBJECTS]->GetEntryByName("..\\..\\DATA\\OBJECTS\\HUD.IFF");
    hud = new RSHud();
    hud->InitFromRam(hud_def->data, hud_def->size);
    for (int i=0; i<18; i++) {
        HudLine line;
        line.start.x = 0;
        line.start.y = 0+i*10;
        line.end.x = 70;
        line.end.y = 0+i*10;
        horizon.push_back(line);
    }
    TreEntry *fontdef = Assets.tres[AssetManager::TRE_MISC]->GetEntryByName("..\\..\\DATA\\FONTS\\HUDFONT.SHP");
    PakArchive *pak = new PakArchive();
    pak->InitFromRAM("HUDFONT", fontdef->data, fontdef->size);
    this->font = new RSFont();
    this->font->InitFromPAK(pak);

}
/**
 * Render the cockpit in its current state.
 *
 * If the face number is non-negative, renders the cockpit in its 2D
 * representation using the VGA graphics mode. Otherwise, renders the
 * cockpit in its 3D representation using the OpenGL graphics mode.
 *
 * @param face The face number of the cockpit to render, or -1 to render
 * in 3D.
 */
void SCCockpit::Render(int face) {
    if (face>=0) {
        VGA.SwithBuffers();
        VGA.Activate();
        VGA.Clear();
        VGA.SetPalette(&this->palette);
        VGA.DrawShape(this->cockpit->ARTP.GetShape(face));
        if (face == 0) {
            for (int i=0; i<18; i++) {
                int top = 10;
                int bottom = 90;
                int left = 125;
                int dec = 0;
                if (horizon[i].start.y-(dec+this->pitch) <bottom && horizon[i].end.y-(dec+this->pitch) <bottom && horizon[i].start.y-(dec+this->pitch) >top && horizon[i].end.y-(dec+this->pitch) >top) {
                    Point2D start;
                    start.x = horizon[i].start.x+left-5;
                    start.y = horizon[i].start.y-(dec+this->pitch);
                    VGA.DrawText(this->font, &start, (char *)std::to_string(i).c_str(), 0, 0, 2,2,2);
                    VGA.line(horizon[i].start.x+left, horizon[i].start.y-(dec+this->pitch), horizon[i].end.x+left, horizon[i].end.y-(dec+this->pitch), 0);
                }
            }
        }
        VGA.VSync();
        VGA.SwithBuffers(); 
    } else {
        Renderer.DrawModel(&this->cockpit->REAL.OBJS,0);
    }
    
}