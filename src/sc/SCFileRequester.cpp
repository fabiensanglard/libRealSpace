#include "precomp.h"
#include <filesystem>


void SCFileRequester::cancel() {
    this->opened = false;
}

SCFileRequester::SCFileRequester(std::function<void(std::string)> callback) {
    this->callback = callback;
    RSImageSet *uiImageSet = new RSImageSet();
    TreEntry *tre = Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\SAVELOAD.SHP");
    PakArchive *pak = new PakArchive();
    pak->InitFromRAM("SAVELOAD.SHP", tre->data, tre->size);
    uiImageSet->InitFromPakArchive(pak,0);
    this->uiImageSet = uiImageSet;
    SCButton *button;

    Point2D frp = {(320-uiImageSet->GetShape(0)->GetWidth())/2, (200-uiImageSet->GetShape(0)->GetHeight())/2};
    Point2D buttonDimension = {75, 12};
    Point2D loadGamePosition = {frp.x+54, frp.y+97};
    Point2D cancelPosition = {frp.x+10, frp.y+112};
    Point2D quitToDosPosition = {frp.x+95, frp.y+112};

    Point2D arrowDimentions = {20, 11};
    Point2D fileUp = {frp.x+9, frp.y+41};
    Point2D fileDown = {frp.x+9, frp.y+83};

    Point2D loadTitlePosition = {frp.x+36, frp.y+6};
    uiImageSet->GetShape(1)->position=loadTitlePosition;

    button = new SCButton();
    button->InitBehavior(std::bind(&SCFileRequester::loadFile, this), loadGamePosition, buttonDimension);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(4);
    button->appearance[SCButton::APR_UP].position=loadGamePosition;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(5);
    button->appearance[SCButton::APR_DOWN].position=loadGamePosition;
    button->SetEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->InitBehavior(std::bind(&SCFileRequester::cancel, this), cancelPosition, buttonDimension);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(6);
    button->appearance[SCButton::APR_UP].position=cancelPosition;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(7);
    button->appearance[SCButton::APR_DOWN].position=cancelPosition;
    button->SetEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->InitBehavior(nullptr, quitToDosPosition, buttonDimension);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(8);
    button->appearance[SCButton::APR_UP].position=quitToDosPosition;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(9);
    button->appearance[SCButton::APR_DOWN].position=quitToDosPosition;
    button->SetEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->InitBehavior(std::bind(&SCFileRequester::fileUp, this), fileUp, arrowDimentions);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(10);
    button->appearance[SCButton::APR_UP].position=fileUp;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(11);
    button->appearance[SCButton::APR_DOWN].position=fileUp;
    button->SetEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->InitBehavior(std::bind(&SCFileRequester::fileDown, this), fileDown, arrowDimentions);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(12);
    button->appearance[SCButton::APR_UP].position=fileDown;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(13);
    button->appearance[SCButton::APR_DOWN].position=fileDown;
    button->SetEnable(true);
    buttons.push_back(button);

    this->font = FontManager.GetFont("..\\..\\DATA\\FONTS\\SM-FONT.SHP");
}
SCFileRequester::~SCFileRequester() {
    delete this->uiImageSet;
}
void SCFileRequester::draw(FrameBuffer *fb) {
    checkButtons();
    checkZones();
    FrameBuffer *fb2 = new FrameBuffer(320, 200);
    fb2->FillWithColor(0);
    RLEShape *shape = this->uiImageSet->GetShape(0);
    fb2->DrawShape(shape);
    
    fb->blitLargeBuffer(fb2->framebuffer, 320, 200, shape->GetLeft(), shape->GetTop(), (320-shape->GetWidth())/2, (200-shape->GetHeight())/2, 182, 131);
    fb->DrawShape(this->uiImageSet->GetShape(1));
    for (auto btn: this->buttons) {
        fb->DrawShape(&btn->appearance[btn->GetAppearance()]);
    }
    Point2D textPos = {
        (320-shape->GetWidth())/2+34, 
        (200-shape->GetHeight())/2+48
    };
    fb->PrintText(this->font, {textPos.x,textPos.y-15}, this->current_file, 0);
    int min_y = textPos.y;
    int max_y = textPos.y+4*8;
    for (auto file: this->files) {
        textPos.y+=8;
        if (textPos.y+texte_x < min_y || textPos.y+texte_x > max_y) {
            continue;
        }
        for (auto z: this->zones) {
            if (z->id == this->selectd_file_index) {
                if (z->position.y+texte_x > min_y && z->position.y+texte_x < max_y) {
                    fb->rect_slow(
                        z->position.x,
                        z->position.y+texte_x,
                        z->position.x+z->dimension.x,
                        z->position.y+z->dimension.y+texte_x,
                        0
                    );
                }
                break;
            }
        }
        fb->PrintText(this->font, {textPos.x, textPos.y+texte_x}, file, 0);
    }
}
void SCFileRequester::checkevents() {
    SDL_Event keybEvents[1];
    int numKeybEvents = SDL_PeepEvents(keybEvents, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < numKeybEvents; i++) {
        SDL_Event *event = &keybEvents[i];

        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: {
            this->opened = false;
            break;
        }
        case SDLK_DELETE:
            case SDLK_BACKSPACE:
                if(this->current_file.size() > 0){
                    this->current_file = this->current_file.substr(0,this->current_file.size()-1);
                }
            break;
            case SDLK_a:
            case SDLK_b:
            case SDLK_c:
            case SDLK_d:
            case SDLK_e:
            case SDLK_f:
            case SDLK_g:
            case SDLK_h:
            case SDLK_i:
            case SDLK_j:
            case SDLK_k:
            case SDLK_l:
            case SDLK_m:
            case SDLK_n:
            case SDLK_o:
            case SDLK_p:
            case SDLK_q:
            case SDLK_r:
            case SDLK_s:
            case SDLK_t:
            case SDLK_u:
            case SDLK_v:
            case SDLK_w:
            case SDLK_x:
            case SDLK_y:
            case SDLK_z:
            {
                this->current_file += char(event->key.keysym.sym - 'a' + 'A');
                break;
            }
            case SDLK_0:
            case SDLK_1:
            case SDLK_2:
            case SDLK_3:
            case SDLK_4:
            case SDLK_5:
            case SDLK_6:
            case SDLK_7:
            case SDLK_8:
            case SDLK_9:
            case SDLK_PERIOD:
            case SDLK_MINUS:
            {
                this->current_file += char(event->key.keysym.sym);
                break;
            }
        default:
            break;
        }
    }
}

SCButton *SCFileRequester::checkButtons(void) {
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
void SCFileRequester::loadFiles() {
    files.clear();
    files.shrink_to_fit();
    zones.clear();
    zones.shrink_to_fit();
    RLEShape *shape = this->uiImageSet->GetShape(0);
    Point2D textPos = {
        (320-shape->GetWidth())/2+32, 
        (200-shape->GetHeight())/2+48
    };
    int idx = 0;
    for (const auto &entry : std::filesystem::directory_iterator(".")) {
        if (entry.is_regular_file()) {
            std::string fileName = entry.path().filename().string();
            if (fileName.size() >= 4 && (fileName.substr(fileName.size() - 4) == ".sav" || fileName.substr(fileName.size() - 4) == ".SAV")) {
                files.push_back(fileName);
                SCZone *zone = new SCZone();
                zone->id = idx;
                zone->position = {textPos.x, textPos.y};
                zone->dimension = {75, 9};
                zone->active = true;
                zone->onclick = std::bind(&SCFileRequester::selectFile, this, std::placeholders::_1, std::placeholders::_2);
                zones.push_back(zone);
                idx++;
                textPos.y += 8;
            }
        }
    }
}
void SCFileRequester::fileUp() {
    this->texte_x += 8;
}
void SCFileRequester::fileDown() {
    this->texte_x -= 8;
}
SCZone * SCFileRequester::checkZones() {
    RLEShape *shape = this->uiImageSet->GetShape(0);
    Point2D textPos = {
        (320-shape->GetWidth())/2+34, 
        (200-shape->GetHeight())/2+48
    };
    int min_y = textPos.y;
    int max_y = textPos.y+4*8;
    if (Mouse.GetPosition().y < min_y || Mouse.GetPosition().y > max_y) {
        return nullptr;
    }
    for (auto zone : this->zones) {
        if (zone->active) {
            if (Mouse.GetPosition().x > zone->position.x &&
                Mouse.GetPosition().x < zone->position.x + zone->dimension.x &&
                Mouse.GetPosition().y > zone->position.y+texte_x &&
                Mouse.GetPosition().y < zone->position.y + zone->dimension.y +texte_x) {
                // HIT !
                Mouse.SetMode(SCMouse::VISOR);

                // If the mouse button has just been released: trigger action.
                if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::RELEASED)
                    zone->onclick(nullptr, zone->id);
                
                return zone;
            }
        }
    }

    Mouse.SetMode(SCMouse::CURSOR);
    return nullptr;
}
void SCFileRequester::selectFile(void *unused, int index) {
    current_file = files[index];
    this->selectd_file_index = index;
}
void SCFileRequester::loadFile() {
    this->requested_file = this->current_file;    
    this->opened = false;
    this->callback(this->requested_file);
}