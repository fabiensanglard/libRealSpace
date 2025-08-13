//
//  IActivity.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "IActivity.h"
#include "GameEngine.h"

char *strtoupper(char *dest, const char *src) {
    char *result = dest;
    while (*dest++ = toupper(*src++))
        ;
    return result;
}
/**
 * @brief Test if a 2D point is inside a quad.
 *
 * This is the "Ray Casting Algorithm" also known as the "Even-Odd Rule Algorithm"
 * (https://en.wikipedia.org/wiki/Point_in_polygon#Ray_casting_algorithm).
 *
 * @param p The point to test.
 * @param quad The quad to test against.
 * @return true if the point is inside the quad, false otherwise.
 */
bool isPointInQuad(const Point2D &p, const std::vector<Point2D *> *quad) {
    int intersections = 0;
    for (size_t i = 0; i < quad->size(); ++i) {
        Point2D a = *quad->at(i);
        Point2D b = *quad->at((i + 1) % quad->size());

        if ((a.y > p.y) != (b.y > p.y)) {
            double atX = (double)(b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x;
            if (p.x < atX) {
                intersections++;
            }
        }
    }
    return (intersections % 2) != 0;
}
IActivity::IActivity() {}

IActivity::~IActivity() {}

void IActivity::SetTitle(const char *title) { Screen->setTitle(title); }

void IActivity::checkKeyboard(void) {
    // Keyboard
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event *event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            Game->StopTopActivity();
            break;
        }
        default:
            break;
        }
    }
}

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
        this->timer = 4200;
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
        VGA.GetFrameBuffer()->DrawShape(&ap);
    }
}