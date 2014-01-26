//
//  SCDogFightMenu.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//


SCDogFightMenu::SCDogFightMenu(){
    
}

SCDogFightMenu::~SCDogFightMenu(){
    
}

void SCDogFightMenu::Init( ){
    
}

//DELETE ME
static void showAllImage(PakArchive* archive){
    
    for(size_t i = 0 ; i < archive->GetNumEntries() ; i ++){
        PakEntry* entry = archive->GetEntry(i);
        
        RLECodex codex ;
        size_t byteRead;
        
        RSImage screen;
        screen.Create("SCREEN", 320, 200);
        bool errorFound = codex.ReadImage(entry->data, &screen, &byteRead);
        
        
        renderer.Clear();
        if (!errorFound){
            
            renderer.Pause();
            while(renderer.IsPaused())
            {
                renderer.DrawImage(&screen, 2);
            
                renderer.Swap();
                renderer.ShowWindow();
                renderer.PumpEvents();
            }
        }
        screen.ClearContent();
    }
    
    
}

void SCDogFightMenu::ParsePalette(PakEntry* entry){
    ByteStream stream(entry->data);
    
    
    stream.ReadUInt32BE();
    
    
    for (size_t i=0; i < 256; i++) {
        Texel texel;
        texel.r = stream.ReadByte();
        texel.g = stream.ReadByte();
        texel.b = stream.ReadByte();
        texel.a = 255;
        this->palette.SetColor(i,&texel);
    }
}

void SCDogFightMenu::Run(void){
    
    renderer.Init(2);
    
    TreArchive tre ;
    tre.InitFromFile("GAMEFLOW.TRE");
    
    TreEntry* objViewIFF = tre.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OBJVIEW.IFF");
    TreEntry* objViewPAK = tre.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OBJVIEW.PAK");
    
    
    IffLexer objToDisplay;
    objToDisplay.InitFromRAM(objViewIFF->data, objViewIFF->size);
    objToDisplay.List(stdout);
    
    PakArchive assets;
    assets.InitFromRAM("OBJVIEW.PAK",objViewPAK->data, objViewPAK->size);
    assets.List(stdout);
    
    PakEntry* paletteEntry = assets.GetEntry(PAK_ID_PALETTE);
    ParsePalette(paletteEntry);
    renderer.SetPalette(&this->palette);
    
    this->palette.Diff(renderer.GetDefaultPalette());
    
    PakEntry* entry0 = assets.GetEntry(PAK_ID_BACKGROUND);
    PakArchive file0;
    file0.InitFromRAM("OBJVIEW.PAK: file PAK_ID_MENU_DYNAMC",entry0->data, entry0->size);
    file0.List(stdout);
    showAllImage(&file0);
    
}