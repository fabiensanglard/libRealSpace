#include "keyboard.h"
#include "EventManager.h"

Keyboard::Keyboard() {
    m_inputSystem = &InputActionSystem::getInstance();
    m_inputSystem->initialize();
}

Keyboard::~Keyboard() {
    // InputActionSystem est un singleton, pas besoin de libérer
}

bool Keyboard::isActionPressed(const InputAction actionId) const {
    return m_inputSystem->isActionPressed(actionId);
}

bool Keyboard::isActionJustPressed(const InputAction actionId) const {
    return m_inputSystem->isActionJustPressed(actionId);
}

bool Keyboard::isActionJustReleased(const InputAction actionId) const {
    return m_inputSystem->isActionJustReleased(actionId);
}

float Keyboard::getActionValue(const InputAction actionId) const {
    return m_inputSystem->getActionValue(actionId);
}

void Keyboard::registerAction(const InputAction actionId) {
    m_inputSystem->registerAction(actionId);
}

void Keyboard::bindKeyToAction(const InputAction actionId, SDL_Scancode key) {
    m_inputSystem->bindInput(actionId, InputBinding(InputType::KEYBOARD, -1, static_cast<int>(key)));
}

void Keyboard::bindMouseButtonToAction(const InputAction actionId, int button) {
    m_inputSystem->bindInput(actionId, InputBinding(InputType::MOUSE_BUTTON, -1, button));
}

void Keyboard::bindGamepadButtonToAction(const InputAction actionId, int gamepadIndex, SDL_GameControllerButton button) {
    m_inputSystem->bindInput(actionId, InputBinding(InputType::GAMEPAD_BUTTON, gamepadIndex, static_cast<int>(button)));
}

void Keyboard::bindGamepadAxisToAction(const InputAction actionId, int gamepadIndex, SDL_GameControllerAxis axis, float scale) {
    m_inputSystem->bindInput(actionId, InputBinding(InputType::GAMEPAD_AXIS, gamepadIndex, static_cast<int>(axis), scale));
}

bool Keyboard::saveActionBindings(const std::string& filename) const {
    return m_inputSystem->saveBindings(filename);
}

bool Keyboard::loadActionBindings(const std::string& filename) {
    return m_inputSystem->loadBindings(filename);
}

void Keyboard::setJoystickDeadzone(float deadzone) {
    EventManager::getInstance().setJoystickDeadzone(deadzone);
}

float Keyboard::getJoystickDeadzone() const {
    return EventManager::getInstance().getJoystickDeadzone();
}

void Keyboard::update() {
    m_inputSystem->update();
}

void Keyboard::startTextInput(const SDL_Rect* inputRect) {
    m_inputSystem->setInputMode(InputActionSystem::InputMode::TEXT_INPUT);
    m_inputSystem->startTextInput(inputRect);
}

void Keyboard::stopTextInput() {
    m_inputSystem->setInputMode(InputActionSystem::InputMode::NORMAL);
    m_inputSystem->stopTextInput();
}
bool Keyboard::isKeyJustPressed(SDL_Scancode key) const {
    return EventManager::getInstance().isKeyJustPressed(key);
}
bool Keyboard::isTextInputActive() const {
    return m_inputSystem->isTextInputActive();
}

// Implémentation de TextEditor

Keyboard::TextEditor::TextEditor(Keyboard* keyboard)
    : m_keyboard(keyboard) {
}

void Keyboard::TextEditor::setText(const std::string& text) {
    m_text = text;
    m_cursorPosition = static_cast<int>(text.length());
}

void Keyboard::TextEditor::setCursorPosition(int position) {
    if (position < 0) {
        m_cursorPosition = 0;
    } else if (position > static_cast<int>(m_text.length())) {
        m_cursorPosition = static_cast<int>(m_text.length());
    } else {
        m_cursorPosition = position;
    }
}

void Keyboard::TextEditor::setActive(bool active, const SDL_Rect* inputRect) {
    if (active == m_active) return;
    
    m_active = active;
    
    if (active) {
        // Configuration des callbacks pour la saisie de texte
        m_keyboard->m_inputSystem->setTextInputCallback([this](const char* text) {
            this->handleTextInput(text);
        });
        
        m_keyboard->m_inputSystem->setSpecialKeyCallback([this](SDL_Keycode key) {
            this->handleSpecialKey(key);
        });
        
        // Activer la saisie de texte
        m_keyboard->startTextInput(inputRect);
    } else {
        // Désactiver la saisie de texte
        m_keyboard->stopTextInput();
    }
}

std::string Keyboard::TextEditor::getDisplayText() const {
    if (!m_active) {
        return m_text;
    }
    
    // Insérer le caractère '_' à la position du curseur
    std::string displayText = m_text;
    displayText.insert(m_cursorPosition, "_");
    return displayText;
}

void Keyboard::TextEditor::update() {
    // Rien à faire ici, tout est géré par les callbacks
}

void Keyboard::TextEditor::handleTextInput(const char* text) {
    // Insérer le texte à la position du curseur
    m_text.insert(m_cursorPosition, text);
    m_cursorPosition += static_cast<int>(strlen(text));
}

void Keyboard::TextEditor::handleSpecialKey(SDL_Keycode key) {
    switch (key) {
        case SDLK_BACKSPACE:
            if (m_cursorPosition > 0) {
                m_text.erase(m_cursorPosition - 1, 1);
                m_cursorPosition--;
            }
            break;
            
        case SDLK_DELETE:
            if (m_cursorPosition < static_cast<int>(m_text.length())) {
                m_text.erase(m_cursorPosition, 1);
            }
            break;
            
        case SDLK_LEFT:
            if (m_cursorPosition > 0) {
                m_cursorPosition--;
            }
            break;
            
        case SDLK_RIGHT:
            if (m_cursorPosition < static_cast<int>(m_text.length())) {
                m_cursorPosition++;
            }
            break;
            
        case SDLK_HOME:
            m_cursorPosition = 0;
            break;
            
        case SDLK_END:
            m_cursorPosition = static_cast<int>(m_text.length());
            break;
            
        case SDLK_ESCAPE:
            setActive(false);
            break;
    }
}

std::shared_ptr<Keyboard::TextEditor> Keyboard::createTextEditor() {
    return std::make_shared<TextEditor>(this);
}