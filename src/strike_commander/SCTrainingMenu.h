//
//  File.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__File__
#define __libRealSpace__File__

class SCTrainingMenu : public IActivity {

public:
    SCTrainingMenu();
    ~SCTrainingMenu();

    void init();
    void unFocus(void);
    void focus(void);
    void runFrame(void);

private:
    RLEShape title;
    RLEShape background;
    RLEShape board;

    void OnExitTraining();
    void OnSearchAndDestroy();
    void OnDogFight();
};

#endif /* defined(__libRealSpace__File__) */
