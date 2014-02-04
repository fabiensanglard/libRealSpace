//
//  main.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

//Here are all the mean subsystems interacting together
GameEngine      Game;
RSScreen        Screen;
RSVGA           VGA;
SCMouse         Mouse;
AssetManager    Assets;
SCRenderer      Renderer;
ConvAssetManager ConvAssets;


int main(int argc, char* argv[]) {

    
    Assets.SetBase("/Users/fabiensanglard/SC/SC/");
    
    Game.Init();
    
    
    //Add MainMenu activity on the game stack.
    //SCMainMenu* main = new SCMainMenu();
    
    for(int i=12 ; i <15 ; i++){
        SCConvPlayer* main = new SCConvPlayer();
        main->Init();
        main->SetID(i);
        Game.AddActivity(main);
    }
    
    Game.Run();
    
    return EXIT_SUCCESS;
}