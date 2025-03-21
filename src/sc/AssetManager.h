//
//  AssetManager.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__AssetManager__
#define __libRealSpace__AssetManager__

class AssetManager{
    
public:
    void SetBase(const char* base);
    void Init(void);
    
    enum TreID {TRE_GAMEFLOW, TRE_OBJECTS, TRE_MISC, TRE_SOUND, TRE_MISSIONS,TRE_TEXTURES } ;
    
    std::vector<TreArchive*> tres;
    
    
    AssetManager();
    ~AssetManager();
    bool ReadISOImage(const std::string& isoPath);
    bool ReadFileFromISO(const std::string& fileName, std::vector<char>& fileData);
    void ListFilesInDirectory(const std::string& directory, std::vector<std::string>& files);

private:
    struct FileEntry {
        bool isDirectory;
        std::vector<char> data;
    };

    bool ExtractFileIndex(std::ifstream& isoFile);

    std::map<std::string, FileEntry> fileContents;

};
#endif /* defined(__libRealSpace__AssetManager__) */
