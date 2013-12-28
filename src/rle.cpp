//
//  rle.cpp
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "rle.h"


#include "ByteStream.h"
#include <stdio.h>





void ReadFragment(ByteStream* stream,RLEFragment* frag){
    
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

bool ExpandFragment(ByteStream* stream,RLEFragment* frag, Texture* dst ){
    
    bool error;
    
	switch(frag->type){
            
		case FRAG_RAW:
		{
			for(int i=0 ; i < frag->numTexels ; i++){
				uint8_t color = stream->ReadByte();
				error = dst->WriteRLETexel(frag->dx+i,frag->dy,color);
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
                        error = dst->WriteRLETexel(frag->dx+numOfTexelsWritten,frag->dy,color);
                        if (error)
                            return true;
                        numOfTexelsWritten++;
                    }
                   
                }
                else{
                    uint8_t color = stream->ReadByte();
                    for(int i=0 ; i < fragNumTexels ; i++){
                        error = dst->WriteRLETexel(frag->dx+numOfTexelsWritten,frag->dy,color);
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



bool ReadImage(uint8_t* src, Texture* dst, size_t* byteRead){
	
	ByteStream stream(src);
	
	RLEFragment frag ;
    
    int16_t rightDist = stream.ReadShort();
    int16_t leftDist  = stream.ReadShort();
    int16_t topDist   = stream.ReadShort();
    int16_t botDist   = stream.ReadShort();

    dst->SetRLECenterCoo(leftDist, rightDist, topDist, botDist);
    
	ReadFragment(&stream,&frag);
    
	while( frag.type != FRAG_END){
        
		bool error = ExpandFragment(&stream,&frag,dst);
        if (error)
            return true;
        
		ReadFragment(&stream,&frag);	
	}
    
    *byteRead = stream.GetPosition()-src;
    
    return false;
}




