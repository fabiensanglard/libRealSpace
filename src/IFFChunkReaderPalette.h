//
//  IFFPaletteReader.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__IFFPaletteReader__
#define __iff__IFFPaletteReader__



class IFFChunkReaderPalette : public IFFChunkReader{
    
public:
    IFFChunkReaderPalette();
    ~IFFChunkReaderPalette();
    
    
    
private:
    
    RealSpacePaletteObject palette;
    
    void ParsePALT(char id[4],uint8_t* data, size_t size);
    void ParseBLWH(char id[4],uint8_t* data, size_t size);
    void ParseRANG(char id[4],uint8_t* data, size_t size);
    
    //MIDGAMES store their palette in a different IFF chunk :( !
    void ParseCMAP(char id[4],uint8_t* data, size_t size);
    
};

#endif /* defined(__iff__IFFPaletteReader__) */
