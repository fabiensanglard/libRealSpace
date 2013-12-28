//
//  IFFChunkReader.h
//  iff
//
//  Created by Fabien Sanglard on 12/27/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __iff__IFFChunkReader__
#define __iff__IFFChunkReader__

class IFFChunkReader;

typedef void (IFFChunkReader::*IDReader)(char chunkID[4],uint8_t* data,size_t size);

class IFFChunkReader{
    
    
public:
    
    virtual bool CanRead(char chunkID[4]) ;
    virtual void Read   (char chunkID[4],uint32_t size,uint8_t* data) ;
    
    IFFChunkReader();
    ~IFFChunkReader();
protected:
    
    
    
    void AddCapability(IDReader idReader,char chunkID[4]);
    
    std::map<uint32_t,IDReader> capabilities;
    
private:
    
    
    
};

#endif /* defined(__iff__IFFChunkReader__) */
