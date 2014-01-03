//
//  rle.cpp
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


#include "ByteStream.h"
#include <stdio.h>

RLECodex::RLECodex(){
    
}


RLECodex::~RLECodex(){
    
}


void RLECodex::ReadFragment(ByteStream* stream,RLEFragment* frag){
    
	uint16_t code = stream->ReadUShort();

    
    if (code == 0){
		frag->type = FRAG_END;
		return;
	}
    
    frag->dx = stream->ReadShort();
	frag->dy = stream->ReadShort();
	
    
	frag->isCompressed = (code & 0x01);
	frag->numTexels =     code >> 1;
    
	if (frag->isCompressed){
		frag->type = FRAG_COMPOSITE;
	}
	else{
		frag->type = FRAG_RAW;
	}
    
	
}

bool RLECodex::ExpandFragment(ByteStream* stream,RLEFragment* frag, RSImage* dst ){
    
    bool error;
    
	switch(frag->type){
            
		case FRAG_RAW:
		{
			for(int i=0 ; i < frag->numTexels ; i++){
				uint8_t color = stream->ReadByte();
				error = WriteRLETexel(dst,frag->dx+i,frag->dy,color);
                if (error)
                    return true;
			}
		}
            break;
            
		case FRAG_COMPOSITE:
		{
            int numOfTexelsWritten =  0;
            
            while (numOfTexelsWritten < frag->numTexels)
            {
                uint8_t subCode = stream->ReadByte();
                uint8_t subCodeType = subCode % 2;
            
                uint16_t fragNumTexels = subCode >> 1;
            
                if (subCodeType == SUB_FRAG_RAW){
                    for(int i=0 ; i < fragNumTexels ; i++){
                        uint8_t color = stream->ReadByte();
                        error = WriteRLETexel(dst,frag->dx+numOfTexelsWritten,frag->dy,color);
                        if (error)
                            return true;
                        numOfTexelsWritten++;
                    }
                   
                }
                else{
                    uint8_t color = stream->ReadByte();
                    for(int i=0 ; i < fragNumTexels ; i++){
                        error = WriteRLETexel(dst,frag->dx+numOfTexelsWritten,frag->dy,color);
                        if (error)
                            return true;

                        numOfTexelsWritten++;
                    }
                }
            }
            
		}
            break;
            
        case FRAG_END:
        {
            return false;
        }
            break;
            
	}
    
    return false;
}


bool RLECodex::WriteRLETexel(RSImage* dstImage,int16_t dx,int16_t dy, uint8_t color){
    
    uint8_t* dst = rleCenter;
    dst+=dx;
    dst+=dy*dstImage->width;
    
    
    
    if (dst < (dstImage->data+dstImage->width*dstImage->height) && dst >= dstImage->data)
        *dst = color;
    else{
        //printf("Error, trying to write outside texture.\n");
        return true;
    }
    
    return false;

}

bool RLECodex::ReadImage(uint8_t* src, RSImage* dst, size_t* byteRead){
	
	ByteStream stream(src);
	
	RLEFragment frag ;
    
    this->rightDist = stream.ReadShort();
    this->leftDist  = stream.ReadShort();
    this->topDist   = stream.ReadShort();
    this->botDist   = stream.ReadShort();
    rleCenter= dst->data + abs(leftDist) + abs(topDist) * dst->width;
    
    
	ReadFragment(&stream,&frag);
    
	while( frag.type != FRAG_END){
        
		bool error = ExpandFragment(&stream,&frag,dst);
        if (error){
            printf("Error:\n");
            return true;
        }
		ReadFragment(&stream,&frag);	
	}
    
    *byteRead = stream.GetPosition()-src;
    
    return false;
}




