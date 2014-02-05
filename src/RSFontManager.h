//
//  SCFontManager.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/5/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCFontManager__
#define __libRealSpace__SCFontManager__

class RSFontManager{
public:
    RSFontManager();
    ~RSFontManager();
    
    void Init(void);
    RSFont* GetFont(const char* name);
    
protected:
private:
    RSFont font;
    std::map<char*,RSFont*,Char_String_Comparator> fonts;
};

#endif /* defined(__libRealSpace__SCFontManager__) */
