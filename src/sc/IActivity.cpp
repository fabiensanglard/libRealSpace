//
//  IActivity.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

char *strtoupper(char *dest, const char *src) {
    char *result = dest;
    while (*dest++ = toupper(*src++))
        ;
    return result;
}

IActivity::IActivity() {}

IActivity::~IActivity() {}

void IActivity::SetTitle(const char *title) { Screen.SetTitle(title); }

SCButton *IActivity::CheckButtons(void) {
    for (size_t i = 0; i < buttons.size(); i++) {

        SCButton *button = buttons[i];

        if (!button->IsEnabled())
            continue;

        if (Mouse.GetPosition().x < button->position.x ||
            Mouse.GetPosition().x > button->position.x + button->dimension.x ||
            Mouse.GetPosition().y < button->position.y ||
            Mouse.GetPosition().y > button->position.y + button->dimension.y) {
            button->SetAppearance(SCButton::APR_UP);
            continue;
        }

        // HIT !
        Mouse.SetMode(SCMouse::VISOR);

        if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::PRESSED)
            button->SetAppearance(SCButton::APR_DOWN);

        // If the mouse button has just been released: trigger action.
        if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::RELEASED)
            button->OnAction();

        return button;
    }

    Mouse.SetMode(SCMouse::CURSOR);
    return NULL;
}

void IActivity::DrawButtons(void) {

    for (auto button : buttons) {
        RLEShape ap = button->appearance[button->GetAppearance()];
        VGA.DrawShape(&ap);
    }
}