//
//  RSImageSet.h
//  libRealSpace
//
//  Created by fabien sanglard on 2/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "PakArchive.h"
#include "RLEShape.h"
#include <stdint.h>
#include <vector>

class RSImageSet {

public:
    RSImageSet();
    ~RSImageSet();

    void InitFromPakEntry(PakEntry *entry);
    void InitFromSubPakEntry(PakArchive *entry);
    void InitFromPakArchive(PakArchive *entry);
    RLEShape *GetShape(size_t index);
    size_t GetNumImages(void);
    std::vector<uint8_t> sequence;
    void Add(RLEShape *shape);

private:
    std::vector<RLEShape *> shapes;
};
