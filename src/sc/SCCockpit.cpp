//
//  SCCockpit.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 02/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include "precomp.h"

Point2D rotateAroundPoint(Point2D point, Point2D center, float angle) {
    float x = (float) (point.x - center.x);
    float y = (float) (point.y - center.y);
    float newx = x * cos(angle) - y * sin(angle) + center.x;
    float newy = x * sin(angle) + y * cos(angle) + center.y;
    return {(int)newx, (int)newy};
}
SCCockpit::SCCockpit() {}
SCCockpit::~SCCockpit() {}
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
void SCCockpit::Init() {
    IffLexer lexer;
    lexer.InitFromFile("PALETTE.IFF");
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();
    cockpit = new RSCockpit();
    TreEntry *cockpit_def =
        Assets.tres[AssetManager::TRE_OBJECTS]->GetEntryByName("..\\..\\DATA\\OBJECTS\\F16-CKPT.IFF");
    cockpit->InitFromRam(cockpit_def->data, cockpit_def->size);

    TreEntry *hud_def = Assets.tres[AssetManager::TRE_OBJECTS]->GetEntryByName("..\\..\\DATA\\OBJECTS\\HUD.IFF");
    hud = new RSHud();
    hud->InitFromRam(hud_def->data, hud_def->size);
    for (int i = 0; i < 36; i++) {
        HudLine line;
        line.start.x = 0;
        line.start.y = 0 + i * 20;
        line.end.x = 70;
        line.end.y = 0 + i * 20;
        horizon.push_back(line);
    }
    TreEntry *fontdef = Assets.tres[AssetManager::TRE_MISC]->GetEntryByName("..\\..\\DATA\\FONTS\\SHUDFONT.SHP");
    PakArchive *pak = new PakArchive();
    pak->InitFromRAM("HUDFONT", fontdef->data, fontdef->size);
    this->font = new RSFont();
    this->font->InitFromPAK(pak);
}
void SCCockpit::RenderHudHorizonLinesSmall() {
    Point2D center;
    center.x = 161;
    center.y = 50;

    std::vector<HudLine> *hline = new std::vector<HudLine>();
    hline->resize(36);
    for (int i = 0; i < 36; i++) {
        hline->at(i).start.x = horizon[i].start.x;
        hline->at(i).start.y = (int) (horizon[i].start.y-(270-this->pitch*4)) % 720;
        if (hline->at(i).start.y < 0) {
            hline->at(i).start.y += 720; 
        }
        hline->at(i).end.x = horizon[i].end.x;
        hline->at(i).end.y = hline->at(i).start.y;
    }
    int ladder=90;
    for (auto h: *hline) {
        int top = 10;
        int bottom = 90;
        int left = 130;
        int dec = 40;
        

        HudLine l = h;
        HudLine l2 = h;
        l.start. x = l.start.x + left;
        l.start. y = l.start.y - dec;
        l.end.y = l.end.y - dec;
        l.end.x = l.start.x + 15;
        
        l2.start. x = l.end.x + 30;
        l2.start. y = l2.start.y - dec;
        l2.end.y = l2.end.y - dec;
        l2.end.x = l2.start.x + 15;

        l.start = rotateAroundPoint(l.start, center, this->roll * M_PI / 180.0f);
        l.end = rotateAroundPoint(l.end, center, this->roll * M_PI / 180.0f);

        l2.start = rotateAroundPoint(l2.start, center, this->roll * M_PI / 180.0f);
        l2.end = rotateAroundPoint(l2.end, center, this->roll * M_PI / 180.0f);
        
        int bx1, bx2, by1, by2;
        bx1 = 125;
        bx2 = 125 + 80;
        by1 = 10;
        by2 = 90;
        if (l.start.x > bx1 && l.start.x < bx2 && l.start.y > by1 && l.start.y < by2) {
            Point2D p = l.start;
            p.y -=2;
            VGA.DrawText(this->font, &p, (char *)std::to_string(ladder).c_str(), 0, 0, 3, 2, 2);
        }
        if (l2.end.x > bx1 && l2.end.x < bx2 && l2.end.y > by1 && l2.end.y < by2) {
            Point2D p = l2.end;
            p.x = p.x - 8;
            p.y -=2;
            VGA.DrawText(this->font, &p, (char *)std::to_string(ladder).c_str(), 0, 0, 3, 2, 2);
        }
        int color = 223;
        if (ladder == 0) {
            HudLine l3 = h;
            l3.start. x = l3.start.x + left-5;
            l3.start. y = l3.start.y - dec;
            l3.end.y = l3.end.y - dec;
            l3.end.x = l3.start.x +65;
            l3.start = rotateAroundPoint(l3.start, center, this->roll * M_PI / 180.0f);
            l3.end = rotateAroundPoint(l3.end, center, this->roll * M_PI / 180.0f);
            VGA.lineWithBox(l3.start.x, l3.start.y, l3.end.x, l3.end.y, color, bx1, bx2, by1, by2);
        } else {
            VGA.lineWithBox(l.start.x, l.start.y, l.end.x, l.end.y, color, bx1, bx2, by1, by2);
            VGA.lineWithBox(l2.start.x, l2.start.y, l2.end.x, l2.end.y, color, bx1, bx2, by1, by2);
        }
        
        ladder=ladder-5;
    }
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
    if (face >= 0) {
        VGA.SwithBuffers();
        VGA.Activate();
        VGA.Clear();
        VGA.SetPalette(&this->palette);
        VGA.DrawShape(this->cockpit->ARTP.GetShape(face));
        if (face == 0) {
            this->RenderHudHorizonLinesSmall();
        }
        VGA.plot_pixel(161, 50, 223);
        VGA.VSync();
        VGA.SwithBuffers();
    } else {
        Renderer.DrawModel(&this->cockpit->REAL.OBJS, 0);
    }
}