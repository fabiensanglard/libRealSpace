#include "precomp.h"
#include "../engine/EventManager.h"
#include <boost/filesystem.hpp>


void SCFileRequester::cancel() {
    this->opened = false;
}

SCFileRequester::SCFileRequester(std::function<void(std::string)> callback, uint8_t shape_id_offset)  {
    this->callback = callback;
    this->shape_id_offset = shape_id_offset;
    RSImageSet *uiImageSet = new RSImageSet();
    TreEntry *tre = Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\SAVELOAD.SHP");
    PakArchive *pak = new PakArchive();
    pak->InitFromRAM("SAVELOAD.SHP", tre->data, tre->size);
    uiImageSet->InitFromPakArchive(pak,0);
    this->uiImageSet = uiImageSet;
    this->color_offset = 49;
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
    button->initBehavior(std::bind(&SCFileRequester::loadFile, this), loadGamePosition, buttonDimension);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(shape_id_offset+4);
    button->appearance[SCButton::APR_UP].position=loadGamePosition;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(shape_id_offset+5);
    button->appearance[SCButton::APR_DOWN].position=loadGamePosition;
    button->setEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->initBehavior(std::bind(&SCFileRequester::cancel, this), cancelPosition, buttonDimension);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(shape_id_offset+6);
    button->appearance[SCButton::APR_UP].position=cancelPosition;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(shape_id_offset+7);
    button->appearance[SCButton::APR_DOWN].position=cancelPosition;
    button->setEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->initBehavior(nullptr, quitToDosPosition, buttonDimension);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(shape_id_offset+8);
    button->appearance[SCButton::APR_UP].position=quitToDosPosition;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(shape_id_offset+9);
    button->appearance[SCButton::APR_DOWN].position=quitToDosPosition;
    button->setEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->initBehavior(std::bind(&SCFileRequester::fileUp, this), fileUp, arrowDimentions);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(shape_id_offset+10);
    button->appearance[SCButton::APR_UP].position=fileUp;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(shape_id_offset+11);
    button->appearance[SCButton::APR_DOWN].position=fileUp;
    button->setEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->initBehavior(std::bind(&SCFileRequester::fileDown, this), fileDown, arrowDimentions);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(shape_id_offset+12);
    button->appearance[SCButton::APR_UP].position=fileDown;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(shape_id_offset+13);
    button->appearance[SCButton::APR_DOWN].position=fileDown;
    button->setEnable(true);
    buttons.push_back(button);

    this->font = FontManager.GetFont("..\\..\\DATA\\FONTS\\SM-FONT.SHP");
    TreEntry *optPalettesEntry = Assets.GetEntryByName(Assets.optpals_filename);
    
    optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
    rawpal = optPals.GetEntry(4)->data;
}

SCFileRequester::SCFileRequester(std::function<void(std::string)> callback, uint8_t shape_id_offset, bool save)  {
    this->callback = callback;
    this->save_mode = save;
    this->shape_id_offset = shape_id_offset;
    RSImageSet *uiImageSet = new RSImageSet();
    TreEntry *tre = Assets.GetEntryByName("..\\..\\DATA\\GAMEFLOW\\SAVELOAD.SHP");
    PakArchive *pak = new PakArchive();
    pak->InitFromRAM("SAVELOAD.SHP", tre->data, tre->size);
    uiImageSet->InitFromPakArchive(pak,0);
    this->uiImageSet = uiImageSet;
    this->color_offset = 49;
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
    button->initBehavior(std::bind(&SCFileRequester::loadFile, this), loadGamePosition, buttonDimension);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(shape_id_offset+2);
    button->appearance[SCButton::APR_UP].position=loadGamePosition;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(shape_id_offset+3);
    button->appearance[SCButton::APR_DOWN].position=loadGamePosition;
    button->setEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->initBehavior(std::bind(&SCFileRequester::cancel, this), cancelPosition, buttonDimension);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(shape_id_offset+6);
    button->appearance[SCButton::APR_UP].position=cancelPosition;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(shape_id_offset+7);
    button->appearance[SCButton::APR_DOWN].position=cancelPosition;
    button->setEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->initBehavior(nullptr, quitToDosPosition, buttonDimension);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(shape_id_offset+8);
    button->appearance[SCButton::APR_UP].position=quitToDosPosition;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(shape_id_offset+9);
    button->appearance[SCButton::APR_DOWN].position=quitToDosPosition;
    button->setEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->initBehavior(std::bind(&SCFileRequester::fileUp, this), fileUp, arrowDimentions);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(shape_id_offset+10);
    button->appearance[SCButton::APR_UP].position=fileUp;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(shape_id_offset+11);
    button->appearance[SCButton::APR_DOWN].position=fileUp;
    button->setEnable(true);
    buttons.push_back(button);

    button = new SCButton();
    button->initBehavior(std::bind(&SCFileRequester::fileDown, this), fileDown, arrowDimentions);
    button->appearance[SCButton::APR_UP]=*uiImageSet->GetShape(shape_id_offset+12);
    button->appearance[SCButton::APR_UP].position=fileDown;
    button->appearance[SCButton::APR_DOWN]=*uiImageSet->GetShape(shape_id_offset+13);
    button->appearance[SCButton::APR_DOWN].position=fileDown;
    button->setEnable(true);
    buttons.push_back(button);

    this->font = FontManager.GetFont("..\\..\\DATA\\FONTS\\SM-FONT.SHP");
    TreEntry *optPalettesEntry = Assets.GetEntryByName(Assets.optpals_filename);
    
    optPals.InitFromRAM("OPTPALS.PAK", optPalettesEntry->data, optPalettesEntry->size);
    rawpal = optPals.GetEntry(4)->data;
}

SCFileRequester::~SCFileRequester() {
    if (this->m_isEditingText) {
        EventManager::getInstance().stopTextInput();
        this->m_isEditingText = false;
    }
    delete this->uiImageSet;
}
void SCFileRequester::draw(FrameBuffer *fb) {
    checkButtons();
    checkZones();
    FrameBuffer *fb2 = new FrameBuffer(320, 200);
    fb2->fillWithColor(0);
    RLEShape *shape = this->uiImageSet->GetShape(shape_id_offset+0);
    fb2->drawShape(shape);
    
    fb->blitLargeBuffer(fb2->framebuffer, 320, 200, shape->GetLeft(), shape->GetTop(), (320-shape->GetWidth())/2, (200-shape->GetHeight())/2, 182, 131);
    if (!this->save_mode) {
        RLEShape *shape2 = this->uiImageSet->GetShape(this->shape_id_offset+1);
        Point2D shape2Pos = {
            0,
            0
        };
        shape2->SetPosition(&shape2Pos);
        fb2->fillWithColor(255);
        fb2->drawShape(shape2);
        fb->blitLargeBuffer(
            fb2->framebuffer,
            320,
            200,
            0,
            0,
            (320-shape->GetWidth())/2+(shape2->GetWidth()/2)-18,
            (200-shape->GetHeight())/2+(shape2->GetHeight()/2)+1,
            107,
            12
        );
    }
    for (auto btn: this->buttons) {
        fb->drawShape(&btn->appearance[btn->getAppearance()]);
    }
    Point2D textPos = {
        (320-shape->GetWidth())/2+34, 
        (200-shape->GetHeight())/2+40
    };
    fb->printText(this->font, {textPos.x,textPos.y-5}, this->current_file, this->color_offset);
    int min_y = textPos.y;
    int max_y = textPos.y+6*8;
    for (auto file: this->files) {
        textPos.y+=8;
        if (textPos.y+texte_x < min_y+6 || textPos.y+texte_x > max_y) {
            continue;
        }
        fb->printText(this->font, {textPos.x, textPos.y+texte_x}, file, this->color_offset);
    }
    textPos.y = min_y;
    int selected = 0;

    for (auto z: this->zones) {
        if (z->id == this->selectd_file_index) {
            selected = -1;
            if (z->position.y+texte_x >= min_y && z->position.y+texte_x < max_y) {
                for (int i=0; i<z->dimension.y; i++) {
                    fb->line(
                        z->position.x,
                        z->position.y+i+texte_x+2,
                        z->position.x+z->dimension.x,
                        z->position.y+i+texte_x+2,
                        0
                    );
                }
                fb->printText(this->font, {textPos.x, z->position.y+texte_x+7}, this->files[z->id], this->color_offset+selected);
            }
            break;
        }
    }
}
void SCFileRequester::checkevents() {
    // Texteingabe ueber den EventManager: Er leert die SDL-Ereigniswarteschlange, SDL_PeepEvents sieht keine Tasten mehr.
    // Der InputActionSystem-Textmodus wird bewusst nicht benutzt (er friert die Maus ein).
    EventManager &em = EventManager::getInstance();
    if (!this->m_isEditingText) {
        // Callbacks eines frueher zerstoerten TextEditors (z.B. aus SCRegister) entfernen, sonst Absturz beim ersten Zeichen
        em.setTextInputCallback(nullptr);
        em.setKeyPressCallback(nullptr);
        em.startTextInput();
        this->m_isEditingText = true;
        return;
    }
    if (em.isKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        this->opened = false;
    }
    if (em.isKeyJustPressed(SDL_SCANCODE_KP_PLUS)) {
        this->color_offset = (this->color_offset + 1) % 256;
    }
    if (em.isKeyJustPressed(SDL_SCANCODE_KP_MINUS)) {
        this->color_offset = (this->color_offset + 255) % 256;
    }
    if (em.isKeyJustPressed(SDL_SCANCODE_KP_MULTIPLY)) {
        this->palette_index = (this->palette_index + 1) % 40;
    }
    if (em.isKeyJustPressed(SDL_SCANCODE_KP_DIVIDE)) {
        this->palette_index = (this->palette_index + 39) % 40;
    }
    if (em.isKeyJustPressed(SDL_SCANCODE_BACKSPACE) || em.isKeyJustPressed(SDL_SCANCODE_DELETE)) {
        if (!this->current_file.empty()) {
            this->current_file.pop_back();
        }
    }
    for (const std::string &txt : em.getTextInput()) {
        for (char c : txt) {
            if (c >= 97 && c <= 122) {
                this->current_file += (char)(c - 97 + 65);
            } else if ((c >= 65 && c <= 90) || (c >= 48 && c <= 57) || c == 46) {
                this->current_file += c;
            }
        }
    }
}

SCButton *SCFileRequester::checkButtons(void) {
    for (size_t i = 0; i < buttons.size(); i++) {

        SCButton *button = buttons[i];

        if (!button->isEnabled())
            continue;

        if (Mouse.getPosition().x < button->position.x ||
            Mouse.getPosition().x > button->position.x + button->dimension.x ||
            Mouse.getPosition().y < button->position.y ||
            Mouse.getPosition().y > button->position.y + button->dimension.y) {
            button->setAppearance(SCButton::APR_UP);
            continue;
        }
        // HIT !
        Mouse.setMode(SCMouse::VISOR);

        if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::PRESSED)
            button->setAppearance(SCButton::APR_DOWN);

        // If the mouse button has just been released: trigger action.
        if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::RELEASED)
            button->onAction();

        return button;
    }

    Mouse.setMode(SCMouse::CURSOR);
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
        (200-shape->GetHeight())/2+40
    };
    int idx = 0;
    for (const auto &entry : boost::filesystem::directory_iterator(".")) {
        if (entry.is_regular_file()) {
            std::string fileName = entry.path().filename().string();
            if (fileName.size() >= 4 && (fileName.substr(fileName.size() - 4) == ".sav" || fileName.substr(fileName.size() - 4) == ".SAV")) {
                files.push_back(fileName);
                SCZone *zone = new SCZone();
                zone->id = idx;
                zone->position = {textPos.x, textPos.y};
                zone->dimension = {120, 9};
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
        (200-shape->GetHeight())/2+40
    };
    int min_y = textPos.y;
    int max_y = textPos.y+6*8;
    if (Mouse.getPosition().y < min_y || Mouse.getPosition().y > max_y) {
        return nullptr;
    }
    for (auto zone : this->zones) {
        if (zone->active) {
            if (Mouse.getPosition().x > zone->position.x &&
                Mouse.getPosition().x < zone->position.x + zone->dimension.x &&
                Mouse.getPosition().y > zone->position.y+texte_x &&
                Mouse.getPosition().y < zone->position.y + zone->dimension.y +texte_x) {
                // HIT !
                Mouse.setMode(SCMouse::VISOR);

                // If the mouse button has just been released: trigger action.
                if (Mouse.buttons[MouseButton::LEFT].event == MouseButton::RELEASED)
                    zone->onclick(nullptr, zone->id);
                
                return zone;
            }
        }
    }

    Mouse.setMode(SCMouse::CURSOR);
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
