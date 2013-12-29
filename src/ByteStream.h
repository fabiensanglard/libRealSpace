//
//  Stream.h
//  pak
//
//  Created by Fabien Sanglard on 12/23/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __pak__Stream__
#define __pak__Stream__

/*
 
 This is the class used to parse all byte streams.
 It is just a class which encapsulate convenient operations
 and can read byte,short,int as Little or Big Endian.
 
*/
class ByteStream{
public:
    

    
    ByteStream(uint8_t* cursor);
    ByteStream(ByteStream& stream);
    ByteStream();
    ~ByteStream();
    
    inline void Set(uint8_t* cursor){
        this->cursor = cursor;
    }
    
    inline void MoveForward(size_t bytes){
        this->cursor += bytes;
    }
    
	inline uint8_t ReadByte(void){
		return *this->cursor++;
	}
    
    inline uint16_t ReadUShort(void){
        uint16_t* ushortP = (uint16_t*)this->cursor;
        this->cursor+=2;
		return *ushortP;
	}
    
    inline int16_t ReadShort(void){
        int16_t* shortP = (int16_t*)this->cursor;
        this->cursor+=2;
		return *shortP;
	}
    
	inline uint8_t* GetPosition(void){
		return this->cursor;
	}
    
    inline uint32_t ReadUInt32LE(void){
        uint32_t* i = (uint32_t*)cursor;
        cursor+=4;
        return *i;
    }
    
    inline uint32_t ReadUInt32BE(void){
        
        uint32_t toLittleEndian = 0;
        toLittleEndian |= *(cursor++)   << 24 ;
        toLittleEndian |= *(cursor++)   << 16 ;
        toLittleEndian |= *(cursor++)   <<  8 ;
        toLittleEndian |= *(cursor++)   <<  0 ;
    
        return toLittleEndian;
    }
    
    
    
private:
	uint8_t* cursor;
};

#endif /* defined(__pak__Stream__) */
