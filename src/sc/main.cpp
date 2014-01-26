//
//  main.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

//Here are all the mean subsystems interacting together
GameEngine    Game;
Renderer      renderer;
SCMouse       Mouse;
AssetManager  Assets;

int main(int argc, char* argv[]) {
    
    Assets.SetBase("/Users/fabiensanglard/SC/SC/");
    
    Game.Init();
    
    
    //Add MainMenu activity on the game stack.
    SCMainMenu* main = new SCMainMenu();
    main->Init();
    Game.AddActivity(main);
    
    Game.Run();
    
    return EXIT_SUCCESS;
}