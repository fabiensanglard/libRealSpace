//
//  IFFObjectReader.h
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__IFFObjectReader__
#define __iff__IFFObjectReader__


class IFFChunkReaderObject : public IFFChunkReader{
    
public:
    IFFChunkReaderObject();
    ~IFFChunkReaderObject();
    RealSpaceObject* GetObject(void);
    
private:
    
    
    void ParseTXMP(char id[4],uint8_t* data, size_t size);
    void ParseVERT(char id[4],uint8_t* data, size_t size);
    void ParseLVL0(char id[4],uint8_t* data, size_t size);
    void ParseVTRI(char id[4],uint8_t* data, size_t size);
    void ParseUVXY(char id[4],uint8_t* data, size_t size);
    
    RealSpaceObject object;
};

#endif /* defined(__iff__IFFObjectReader__) */
