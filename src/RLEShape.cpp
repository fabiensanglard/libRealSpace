//
//  rle.cpp
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


RLEShape::RLEShape()

{
    position.x = 0;
    position.y = 0;
}


RLEShape::~RLEShape(){
    
}


void RLEShape::ReadFragment(RLEFragment* frag){
    
	uint16_t code = stream.ReadUShort();

    
    if (code == 0){
		frag->type = FRAG_END;
		return;
	}
    
    frag->dx = stream.ReadShort();
	frag->dy = stream.ReadShort();
	
    
	frag->isCompressed = (code & 0x01);
	frag->numTexels =     code >> 1;
    
	if (frag->isCompressed){
		frag->type = FRAG_COMPOSITE;
	}
	else{
		frag->type = FRAG_RAW;
	}
    
	
}

bool RLEShape::ExpandFragment(RLEFragment* frag, uint8_t* dst ){
    
    bool error;
    
	switch(frag->type){
            
		case FRAG_RAW:
		{
			for(int i=0 ; i < frag->numTexels ; i++){
				uint8_t color = stream.ReadByte();
				error = WriteColor(dst,frag->dx+i,frag->dy,color);
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
                uint8_t subCode = stream.ReadByte();
                uint8_t subCodeType = subCode % 2;
            
                uint16_t fragNumTexels = subCode >> 1;
            
                if (subCodeType == SUB_FRAG_RAW){
                    for(int i=0 ; i < fragNumTexels ; i++){
                        uint8_t color = stream.ReadByte();
                        error = WriteColor(dst,frag->dx+numOfTexelsWritten,frag->dy,color);
                        if (error)
                            return true;
                        numOfTexelsWritten++;
                    }
                   
                }
                else{
                    uint8_t color = stream.ReadByte();
                    for(int i=0 ; i < fragNumTexels ; i++){
                        error = WriteColor(dst,frag->dx+numOfTexelsWritten,frag->dy,color);
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

void RLEShape::Init(uint8_t* idata, size_t isize){
    stream.Set(idata);
    this->size = isize;
    this->data = idata;
    
    /*this->rightDist=*/  stream.ReadShort();
    this->leftDist  = stream.ReadShort();
    this->topDist   = stream.ReadShort();
    /*this->botDist=*/    stream.ReadShort();
    /*rleCenter= dst->data + abs(leftDist) + abs(topDist) * dst->width;*/
    
    data = stream.GetPosition();
}


void RLEShape::InitWithPosition(uint8_t* idata, size_t isize,Point2D* position ){
    Init(idata,isize);
    SetPosition(position);
}

bool RLEShape::Expand(uint8_t* dst, size_t* byteRead){
	
	stream.Set(data);
	
	RLEFragment frag ;
    
    
    
	ReadFragment(&frag);
    
	while( frag.type != FRAG_END){
        
		bool error = ExpandFragment(&frag,dst);
        if (error){
            printf("Error in RLE\n");
            return true;
        }
		ReadFragment(&frag);
	}
    
    *byteRead = stream.GetPosition()-data;
    
    return false;
}


bool RLEShape::WriteColor(uint8_t* dst,int16_t dx, int16_t dy, uint8_t color){
    
    uint8_t* finalDest = dst + leftDist + topDist * 320 + dx + dy * 320 +position.x + position.y * 320;
    
    if (finalDest < dst || finalDest >= dst+(320*200))
        return true;
    
    *finalDest = color;
    
    return false;
}




