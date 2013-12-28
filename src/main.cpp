//
//  main.c
//  iff
//
//  Created by Fabien Sanglard on 12/14/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#define BASE "/Users/fabiensanglard/Desktop/DATA/MIDGAMES"


#define FILE "A-10.IFF"
//#define FILE "DCHUTE.IFF"
//#define FILE "F16PITXP.IFF"  //<--- F16 cocpit !!!
//#define FILE "EJECSEAT.IFF"
//#define FILE "F-22.IFF"
//#define FILE "F-16DES.IFF"

int testIFF(int argc, const char * argv[])
{
    
    IMG_Init();
    
    /*
     IFFReader reader ;
     reader.InitWithFile(BASE "/" FILE);
     reader.Parse();
    */
    
    //return 0;
    //CREDIT
    /*
    IFFReader reader ;
    reader.InitWithFile("/Users/fabiensanglard/Library/Developer/Xcode/DerivedData/StrikeCommanderExplorer-enxcepcbaghljtbacncgzdlmsixa/Build/DATA/MIDGAMES/CREDITS.IFF");
    reader.Parse();
    */
    
    //MAURIT
    /*
    IFFReader reader ;
    reader.InitWithFile("/Users/fabiensanglard/Library/Developer/Xcode/DerivedData/StrikeCommanderExplorer-enxcepcbaghljtbacncgzdlmsixa/Build/DATA/MISSIONS/MAURITAN.IFF");
    reader.Parse();
    */
    
    /*
    IFFReader reader ;
    reader.InitWithFile("/Users/fabiensanglard/Library/Developer/Xcode/DerivedData/StrikeCommanderExplorer-enxcepcbaghljtbacncgzdlmsixa/Build/DATA/GAMEFLOW/OPTIONS.IFF");
    reader.Parse();
    
    
    exit(0);
    */
    /*
    IFFPalettetReader paletteReader;
    paletteReader.InitWithFile("/Users/fabiensanglard/Desktop/SC/strikecommander/SC/"
                               "PALETTE.IFF");
    paletteReader.Parse();
    
    
    
    
    */
    Palette* colorPalette;
    
    //colorPalette= paletteReader.GetObject()->GetColorPalette();
 //   IMG_ShowPalette(colorPalette,16);
    
    
    /*
    const char* iffPath = BASE "/" FILE;
    IFFObjectReader objIFFReader ;
    objIFFReader.InitWithFile(iffPath);
    objIFFReader.Parse();
    
    RealSpaceObject* obj = objIFFReader.GetObject();
    
    
    for (int i = 0 ; i < obj->numTextures ; i++){
        Texture* texture = &obj->textures[i];
       IMG_ShowImage(texture->data, texture->width, texture->height, colorPalette,4,true);
    }
    
    IMG_ShowModel(obj, colorPalette);
    */
    
    return 0;
}

