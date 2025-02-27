//
//  SCFontManager.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/5/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSFontManager.h"


RSFontManager::RSFontManager(){
    
}

RSFontManager::~RSFontManager(){
    
}

void RSFontManager::Init(void){
    
    const char* font_index[31] = {
        "..\\..\\DATA\\FONTS\\CALCFONT.SHP",
        "..\\..\\DATA\\FONTS\\CAMROPT.SHP",
        "..\\..\\DATA\\FONTS\\CHKEXIT.SHP",
        "..\\..\\DATA\\FONTS\\COCKOPT.SHP",
        "..\\..\\DATA\\FONTS\\CONVFONT.SHP",
        "..\\..\\DATA\\FONTS\\DETLOPT.SHP",
        "..\\..\\DATA\\FONTS\\FLITOPT.SHP",
        "..\\..\\DATA\\FONTS\\FORMFONT.SHP",
        "..\\..\\DATA\\FONTS\\GAMEOPT.SHP",
        "..\\..\\DATA\\FONTS\\GRAVFONT.SHP",
        "..\\..\\DATA\\FONTS\\HUDFONT.SHP",
        "..\\..\\DATA\\FONTS\\JOYCALIB.SHP",
        "..\\..\\DATA\\FONTS\\LEDGFONT.SHP",
        "..\\..\\DATA\\FONTS\\MAINOPT.SHP",
        "..\\..\\DATA\\FONTS\\MAPFONT.SHP",
        "..\\..\\DATA\\FONTS\\MENBGRND.SHP",
        "..\\..\\DATA\\FONTS\\MENFONT.SHP",
        "..\\..\\DATA\\FONTS\\MTYPEBUT.SHP",
        "..\\..\\DATA\\FONTS\\OPTFONT.SHP",
        "..\\..\\DATA\\FONTS\\OVBKGRND.SHP",
        "..\\..\\DATA\\FONTS\\OVBUTUD.SHP",
        "..\\..\\DATA\\FONTS\\OVPLAT.SHP",
        "..\\..\\DATA\\FONTS\\RESTART.SHP",
        "..\\..\\DATA\\FONTS\\SHUDFONT.SHP",
        "..\\..\\DATA\\FONTS\\SM-FONT.SHP",
        "..\\..\\DATA\\FONTS\\TR1PLAT.SHP",
        "..\\..\\DATA\\FONTS\\TR2BUTUD.SHP",
        "..\\..\\DATA\\FONTS\\TR2PLAT.SHP",
        "..\\..\\DATA\\FONTS\\3DSTUFON.SHP",
        "..\\..\\DATA\\FONTS\\AUDIOPT.SHP",
        "..\\..\\DATA\\FONTS\\BOARDFNT.SH"
    };
    

    for (int i = 0; i < 30; i++){
        const char* fontPath = font_index[i];
        const char* trePath = "MISC.TRE";
        TreArchive MISC;
        MISC.InitFromFile(trePath);
        TreEntry* convFontEntry = MISC.GetEntryByName(fontPath);
        PakArchive fontArch;
        fontArch.InitFromRAM(font_index[i],convFontEntry->data,convFontEntry->size);
        RSFont *font = new RSFont();
        font->InitFromPAK(&fontArch);
        fonts[fontPath] = font;
    }
}

RSFont* RSFontManager::GetFont(const char* name){
    return fonts[name];
}

