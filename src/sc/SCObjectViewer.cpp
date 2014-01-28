//
//  NSObjectViewer.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

static const uint8_t PAK_ID_MENU_STATIC       = 0;
static const uint8_t PAK_ID_MENU_DYNAMC       = 4;


static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_BACKGROUND_RELEASED   = 0;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_BACKGROUND_PRESSED    = 1;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW_RELEASED      = 2;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW_PRESSED       = 3;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWNARROW_RELEASED    = 4;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWNARROW_PRESSED     = 5;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW2_RELEASED     = 6;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW2_PRESSED      = 7;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWN2ARROW_RELEASED   = 8;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWN2ARROW_PRESSED    = 9;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_LEFTARROW_RELEASED    =10;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_LEFTARROW_PRESSED     =11;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_RIGHTARROW_RELEASED   =12;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_RIGHTARROW_PRESSED    =13;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_EXIT_RELEASED         =14;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_EXIT_PRESSED          =15;

static const uint8_t PAK_ID_BACKGROUND  = 8;

SCObjectViewer::SCObjectViewer(){
    
}

SCObjectViewer::~SCObjectViewer(){
    
}

void ConvertToUpperCase(char *sPtr)
{
    while(*sPtr != '\0')
    {
        if (islower(*sPtr))
            *sPtr = toupper(*sPtr);
        sPtr++;
    }
}

void SCObjectViewer::ListObjects(void){
    
    printf("Listing objects in RSObjecViewer.\n");
    for(size_t i=0 ; i < showCases.size() ; i++){
        RSShowCase showCase = showCases[i];
        printf("%-20s : dist=%.2f\n",showCase.displayName,showCase.cameraDist);
    }
}

void SCObjectViewer::ParseObjList(IffLexer* lexer){
    
    //The objects referenced are within this TRE archive
    TreArchive tre ;
    tre.InitFromFile("OBJECTS.TRE");
    
    //The object all follow the same path:
    const char* OBJ_PATH = "..\\..\\DATA\\OBJECTS\\";
    const char* OBJ_EXTENSION = ".IFF";
    
    IffChunk* chunk = lexer->GetChunkByID('OBJS');
    if (chunk == NULL){
        printf("**Error: Cannot parse Object List (Missing OBJS chunk).\n");
        return;
    }
    
    ByteStream stream(chunk->data);
    
    size_t numObjectInList = chunk->size / 33;
    
    
    for(int objIndex=0 ; objIndex < numObjectInList ; objIndex++){
    
        RSShowCase showCase;
        
        
        char objName[9];
        for(int k = 0 ; k < 9 ; k++)
            objName[k] = stream.ReadByte();
        ConvertToUpperCase(objName);

        
        
        for(int k = 0 ; k < 20 ; k++)
            showCase.displayName[k] = stream.ReadByte();
        
        
        char modelPath[512];
        strcpy(modelPath,OBJ_PATH);
        strcat(modelPath,objName);
        strcat(modelPath, OBJ_EXTENSION);
        TreEntry* entry = tre.GetEntryByName(modelPath);
        
        if (entry == NULL){
            printf("Object reference '%s' not found in TRE.\n",modelPath);
            continue;
        }
            
        showCase.entity = new RSEntity();
        showCase.entity->InitFromRAM(entry->data,entry->size);
        
        uint32_t fixedPointDist = stream.ReadInt32LE();
        showCase.cameraDist = (fixedPointDist >> 8) + (fixedPointDist & 0xFF)/255.0f ;
        //showCase.cameraDist = 200000;
        
        showCases.push_back(showCase);
        
    }
    
    
    
}

void SCObjectViewer::ParseAssets(PakArchive* archive){
    
    
    
    /*
    PakEntry* entry0 = archive->GetEntry(PAK_ID_MENU_DYNAMC);
    PakArchive file0;
    file0.InitFromRAM("OBJVIEW.PAK: file PAK_ID_MENU_DYNAMC",entry0->data, entry0->size);
    file0.List(stdout);
    showAllImage(&file0);
    */
    
    //Identified as OBJECT VIEWER STATIC TITLE
    
    PakEntry* entry0 = archive->GetEntry(0);
    PakArchive file0;
    file0.InitFromRAM("OBJVIEW.PAK: file 0",entry0->data, entry0->size);
    title.Init(file0.GetEntry(0)->data, file0.GetEntry(0)->size);

    
    //Identified as TRAINING MISSION TITLE
    /*
    PakEntry* entry1 = archive->GetEntry(1);
    PakArchive file1;
    file1.InitFromRAM("OBJVIEW.PAK: file 1",entry1->data, entry1->size);
    file1.List(stdout);
    showAllImage(&file1);
    */
    
    //Identified as DOGFIGHT SETUP
    /*
    PakEntry* entry2 = archive->GetEntry(2);
    PakArchive file2;
    file2.InitFromRAM("OBJVIEW.PAK: file 2",entry2->data, entry2->size);
    file2.List(stdout);
    showAllImage(&file2);
    */
    
    //Identified as BUTTONS DODGE FIGHT
    /*
    PakEntry* entry3 = archive->GetEntry(3);
    PakArchive file3;
    file3.InitFromRAM("OBJVIEW.PAK: file 3",entry3->data, entry3->size);
    file3.List(stdout);
    showAllImage(&file3);
    */
    
    //Identified as BUTTONS OBJ VIEWER
    /*
    PakEntry* entry4 = archive->GetEntry(4);
    PakArchive file4;
    file4.InitFromRAM("OBJVIEW.PAK: file 4",entry4->data, entry4->size);
    file4.List(stdout);
    showAllImage(&file4);
    */
    
    //Identified as DODGE FIGHT MISSION BUILDER
    /*
    PakEntry* entry5 = archive->GetEntry(5);
    PakArchive file5;
    file5.InitFromRAM("OBJVIEW.PAK: file 5",entry5->data, entry5->size);
    file5.List(stdout);
    showAllImage(&file5);
    */
    
    //Identified as Dodge Fight background
    /*
    PakEntry* entry6 = archive->GetEntry(6);
    PakArchive file6;
    file6.InitFromRAM("OBJVIEW.PAK: file 6",entry6->data, entry6->size);
    file6.List(stdout);
    showAllImage(&file6);
    */
    
    //Identified as DOGFIGHT PALETTE
    /*
    PakEntry* entry7 = archive->GetEntry(7);
    PakArchive file7;
    file7.InitFromRAM("OBJVIEW.PAK: file 7",entry7->data, entry7->size);
    file7.List(stdout);
    //showAllImage(&file7);
    */
    
    //Identified as blue background
    
    PakEntry* entry8 = archive->GetEntry(8);
    PakArchive file8;
    file8.InitFromRAM("OBJVIEW.PAK: file 8",entry8->data, entry8->size);
    bluePrint.Init(file8.GetEntry(0)->data, file8.GetEntry(0)->size);
    
    
    
    //Unknown content
    /*
    PakEntry* entry9 = archive->GetEntry(9);
    PakArchive file9;
    file9.InitFromRAM("OBJVIEW.PAK: file 9",entry9->data, entry9->size);
    file9.List(stdout);
    showAllImage(&file9);
    */
    
    
    IffLexer lexer ;
    lexer.InitFromFile("PALETTE.IFF");
    //lexer.List(stdout);
    
    RSPalette palette;
    palette.InitFromIFF(&lexer);
    this->palette = *palette.GetColorPalette();
}

void SCObjectViewer::Init(void){
    
    
    TreArchive tre ;
    tre.InitFromFile("GAMEFLOW.TRE");
    
    TreEntry* objViewIFF = tre.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OBJVIEW.IFF");
    TreEntry* objViewPAK = tre.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OBJVIEW.PAK");
    
    
    IffLexer objToDisplay;
    objToDisplay.InitFromRAM(objViewIFF->data, objViewIFF->size);
    objToDisplay.List(stdout);
    ParseObjList(&objToDisplay);
    ListObjects();
    
    
    PakArchive assets;
    assets.InitFromRAM("OBJVIEW.PAK",objViewPAK->data, objViewPAK->size);
    assets.List(stdout);
    //assets.Decompress("/Users/fabiensanglard/Desktop/ObjViewer.PAK", "MEH");
    ParseAssets(&assets);
    
    SetTitle("Neo Object Viewer");
}

void SCObjectViewer::RunFrame(void){
    
    /*
    
    TreArchive tre ;
    tre.InitFromFile("GAMEFLOW.TRE");
    TreEntry* objViewPAK = tre.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OBJVIEW.PAK");
    
    PakArchive assets;
    assets.InitFromRAM("OBJVIEW.PAK",objViewPAK->data, objViewPAK->size);
    
    RLECodex codex ;
    size_t byteRead;
    bool errorFound;
    
    //Get blue background
    PakEntry* backgroundPAKEntry = assets.GetEntry(PAK_ID_BACKGROUND);
    PakArchive backgroundPAK;
    backgroundPAK.InitFromRAM("PAK_ID_BACKGROUND",backgroundPAKEntry->data, backgroundPAKEntry->size);
    RSImage backgroundImage;
    backgroundImage.Create("backgroundImage", 320, 200);
    errorFound = codex.ReadImage(backgroundPAK.GetEntry(0)->data, &backgroundImage, &byteRead);
        
    //Get static manu
    PakEntry* staticMenuPAKEntry = assets.GetEntry(PAK_ID_MENU_STATIC);
    PakArchive staticMenuPAK;
    staticMenuPAK.InitFromRAM("PAK_ID_MENU_STATIC",staticMenuPAKEntry->data, staticMenuPAKEntry->size);
    RSImage staticMenuImage;
    staticMenuImage.Create("staticMenuImage", 320, 200);
    errorFound = codex.ReadImage(staticMenuPAK.GetEntry(0)->data, &staticMenuImage, &byteRead);
    */
    
    
    
    
    /*
    PakEntry* menuBGData = assets.GetEntry(PAK_ID_MENU_DYNAMC);
    */
    
    CheckButtons();
    
    VGA.Activate();
    VGA.Clear();
    
    VGA.SetPalette(&this->palette);
    
    //Draw static
    VGA.DrawShape(&bluePrint);
    VGA.DrawShape(&title);
    
    //VGA.DrawShape(&board);
    
    
    DrawButtons();
    
    //Draw Mouse
    Mouse.Draw();
    
    //Check Mouse state.
    
    VGA.VSync();

    
    /*
    RSShowCase showCase = showCases[0];
    printf("F-16 dimensions: x: %.2f y: %.2f z: %.2f\n",
           showCase.entity->GetBoudingBpx()->max.x-showCase.entity->GetBoudingBpx()->min.x,
           showCase.entity->GetBoudingBpx()->max.y-showCase.entity->GetBoudingBpx()->min.y,
           showCase.entity->GetBoudingBpx()->max.z-showCase.entity->GetBoudingBpx()->min.z
           );
    */
    
    /*
    
    while(1){
        
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        
        uint32_t currentTime = SDL_GetTicks();
        uint32_t totalTime = currentTime - startTime;
        
        modelIndex = (totalTime /4000) % showCases.size();
        
        renderer.Clear();
        renderer.DrawImage(&backgroundImage);
        renderer.DrawImage(&staticMenuImage);
    
        RSShowCase showCase = showCases[modelIndex];
        
        Point3D newPosition;
        newPosition.x= showCase.cameraDist/200 *cos(counter/4);
        newPosition.y= 10;
        newPosition.z= showCase.cameraDist/200*sin(counter/4);
        counter += 0.02;
        
        renderer.GetCamera()->SetPosition(&newPosition);
        Point3D lookAt = {0,0,0};
        renderer.GetCamera()->LookAt(&lookAt);
        
        Point3D light;
        light.x= 20*cos(-counter/2.0f);
        light.y= 10;
        light.z= 20*sin(-counter/2.0f);
        renderer.SetLight(&light);

        glMatrixMode(GL_PROJECTION);
        Matrix* projection = renderer.GetCamera()->GetProjectionMatrix();
        glLoadMatrixf(projection->ToGL());
        
        glMatrixMode(GL_MODELVIEW);
        Matrix* view = renderer.GetCamera()->GetViewMatrix();
        glLoadMatrixf(view->ToGL());
        
        
        
        
        RSEntity* modelToDraw = showCase.entity;
        renderer.DrawModel(modelToDraw, LOD_LEVEL_MAX);
    
        renderer.Swap();
        renderer.ShowWindow();
        renderer.PumpEvents();
    }
    */
    
}