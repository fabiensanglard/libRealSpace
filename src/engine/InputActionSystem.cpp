#include "InputActionSystem.h"
#include "EventManager.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_set>

InputActionSystem& InputActionSystem::getInstance() {
    static InputActionSystem instance;
    return instance;
}

InputActionSystem::InputActionSystem() {
}

InputActionSystem::~InputActionSystem() {
    shutdown();
}

void InputActionSystem::initialize() {
    m_eventManager = &EventManager::getInstance();
    m_eventManager->initialize();
}

void InputActionSystem::shutdown() {
}

void InputActionSystem::update() {
    // Toujours mettre à jour l'EventManager
    m_eventManager->update();

    // Sauvegarder l'état précédent
    for (auto& [actionId, state] : m_actions) {
        state.previousState = state.currentState;
        state.previousValue = state.currentValue;
        state.currentState = false;
        state.currentValue = 0.0f;
    }

    // Whitelist des actions actives pendant TEXT_INPUT
    static const std::unordered_set<InputAction> textWhitelist = {
        InputAction::KEY_TAB,
        InputAction::KEY_RETURN,
        InputAction::KEY_ESCAPE,
        InputAction::MOUSE_LEFT,
        InputAction::MOUSE_POS_X,
        InputAction::MOUSE_POS_Y
    };

    bool textMode = (m_inputMode == InputMode::TEXT_INPUT);

    for (auto& [actionId, state] : m_actions) {
        // En mode texte : ignorer les actions non whitelistes (mais previousState déjà mis à jour)
        if (textMode && textWhitelist.find(actionId) == textWhitelist.end()) {
            continue;
        }

        float highestValue = 0.0f;
        bool any = false;
        for (const auto& binding : state.bindings) {
            float value = 0.0f;
            if (evaluateBinding(binding, value)) {
                any = true;
                if (std::fabs(value) > std::fabs(highestValue))
                    highestValue = value;
            }
        }
        state.currentState  = any;
        state.currentValue  = highestValue;
    }
}

void InputActionSystem::registerAction(const InputAction actionId) {
    // Crée l'action si elle n'existe pas déjà
    if (m_actions.find(actionId) == m_actions.end()) {
        m_actions[actionId] = ActionState();
    }
}

void InputActionSystem::bindInput(const InputAction actionId, const InputBinding& binding) {
    // Assurer que l'action existe
    if (m_actions.find(actionId) == m_actions.end()) {
        registerAction(actionId);
    }
    
    // Ajouter la liaison
    m_actions[actionId].bindings.push_back(binding);
}

void InputActionSystem::unbindInput(const InputAction actionId, const InputBinding& binding) {
    auto it = m_actions.find(actionId);
    if (it != m_actions.end()) {
        auto& bindings = it->second.bindings;
        
        // Supprimer les liaisons correspondantes
        bindings.erase(
            std::remove_if(bindings.begin(), bindings.end(),
                [&binding](const InputBinding& b) {
                    return b.type == binding.type && 
                           b.deviceId == binding.deviceId && 
                           b.code == binding.code;
                }),
            bindings.end()
        );
    }
}

void InputActionSystem::clearBindings(const InputAction actionId) {
    auto it = m_actions.find(actionId);
    if (it != m_actions.end()) {
        it->second.bindings.clear();
    }
}

bool InputActionSystem::isActionPressed(const InputAction actionId) const {
    auto it = m_actions.find(actionId);
    if (it != m_actions.end()) {
        return it->second.currentState;
    }
    return false;
}

bool InputActionSystem::isActionJustPressed(const InputAction actionId) const {
    auto it = m_actions.find(actionId);
    if (it != m_actions.end()) {
        return it->second.currentState && !it->second.previousState;
    }
    return false;
}

bool InputActionSystem::isActionJustReleased(const InputAction actionId) const {
    auto it = m_actions.find(actionId);
    if (it != m_actions.end()) {
        return !it->second.currentState && it->second.previousState;
    }
    return false;
}

float InputActionSystem::getActionValue(const InputAction actionId) const {
    auto it = m_actions.find(actionId);
    if (it != m_actions.end()) {
        return it->second.currentValue;
    }
    return 0.0f;
}

bool InputActionSystem::evaluateBinding(const InputBinding& binding, float& outValue) const {
    outValue = 0.0f;
    
    switch (binding.type) {
        case InputType::KEYBOARD: {
            SDL_Scancode scancode = static_cast<SDL_Scancode>(binding.code);
            bool isPressed = m_eventManager->isKeyPressed(scancode);
            outValue = isPressed ? 1.0f : 0.0f;
            return isPressed;
        }
        
        case InputType::MOUSE_BUTTON: {
            bool isPressed = m_eventManager->isMouseButtonPressed(binding.code);
            outValue = isPressed ? 1.0f : 0.0f;
            return isPressed;
        }
        
        case InputType::MOUSE_AXIS: {
            // Code 0 = X, Code 1 = Y
            int x, y, dx, dy;
            m_eventManager->getMousePosition(x, y);
            m_eventManager->getMouseMotion(dx, dy);
            
            if (binding.code == 0) {
                outValue = static_cast<float>(dx) * binding.scale;
            } else if (binding.code == 1) {
                outValue = static_cast<float>(dy) * binding.scale;
            }
            
            return std::abs(outValue) > m_axisDeadZone;
        }
        case InputType::MOUSE_POSITION: {
            int x, y;
            m_eventManager->getMousePosition(x, y);
            outValue = (binding.code == 0) ? static_cast<float>(x) : static_cast<float>(y);
            return true; // Always true as we can always get mouse position
        }
        case InputType::GAMEPAD_BUTTON: {
            int gamepadId = (binding.deviceId >= 0) ? binding.deviceId : 0;
            if (gamepadId < m_eventManager->getJoystickCount()) {
                SDL_GameControllerButton button = static_cast<SDL_GameControllerButton>(binding.code);
                bool isPressed = m_eventManager->isGamepadButtonPressed(gamepadId, button);
                outValue = isPressed ? 1.0f : 0.0f;
                return isPressed;
            }
            return false;
        }
        
        case InputType::GAMEPAD_AXIS: {
            int gamepadId = (binding.deviceId >= 0) ? binding.deviceId : 0;
            if (gamepadId < m_eventManager->getJoystickCount()) {
                SDL_GameControllerAxis axis = static_cast<SDL_GameControllerAxis>(binding.code);
                float value = m_eventManager->getGamepadAxisValue(gamepadId, axis) * binding.scale;
                
                // Appliquer la zone morte
                if (std::abs(value) < m_axisDeadZone) {
                    value = 0.0f;
                } else {
                    // Normaliser après application de la zone morte
                    float sign = (value > 0) ? 1.0f : -1.0f;
                    value = sign * (std::abs(value) - m_axisDeadZone) / (1.0f - m_axisDeadZone);
                }
                
                outValue = value;
                return std::abs(value) > 0.0f;
            }
            return false;
        }
    }
    
    return false;
}

bool InputActionSystem::saveBindings(const std::string &filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& [actionId, state] : m_actions) {
        for (const auto& binding : state.bindings) {
            file << static_cast<int>(actionId) << "," 
                 << static_cast<int>(binding.type) << "," 
                 << binding.deviceId << "," 
                 << binding.code << "," 
                 << binding.scale << std::endl;
        }
    }
    
    return true;
}

bool InputActionSystem::loadBindings(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Effacer toutes les liaisons actuelles
    for (auto& [actionId, state] : m_actions) {
        state.bindings.clear();
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        InputAction actionId;
        int type, deviceId, code;
        float scale;
        
        // Format: actionId,type,deviceId,code,scale
        std::string actionIdStr;
        if (std::getline(ss, actionIdStr, ',')) {
            actionId = static_cast<InputAction>(std::stoi(actionIdStr));
            ss >> type;
            ss.ignore(); // ignorer la virgule
            ss >> deviceId;
            ss.ignore();
            ss >> code;
            ss.ignore();
            ss >> scale;
            
            registerAction(actionId);
            bindInput(actionId, InputBinding(static_cast<InputType>(type), deviceId, code, scale));
        }
    }
    
    return true;
}
void InputActionSystem::setInputMode(InputMode mode) {
    if (mode == m_inputMode) return;
    
    m_inputMode = mode;
    
    if (mode == InputMode::TEXT_INPUT) {
        startTextInput();
    } else {
        stopTextInput();
    }
}

void InputActionSystem::startTextInput(const SDL_Rect* inputRect) {
    m_eventManager->startTextInput(inputRect);
    
    // Configurer les callbacks de texte
    m_eventManager->setTextInputCallback([this](const char* text) {
        if (m_textInputCallback) {
            m_textInputCallback(text);
        }
    });
    
    m_eventManager->setTextEditingCallback([this](const char* text, int start, int length) {
        if (m_textEditingCallback) {
            m_textEditingCallback(text, start, length);
        }
    });
    
    m_eventManager->setKeyPressCallback([this](SDL_Keycode keycode, Uint16 mod) {
        handleSpecialKeys(keycode);
    });
}

void InputActionSystem::stopTextInput() {
    this->m_inputMode = InputMode::NORMAL;
    m_eventManager->stopTextInput();
}

bool InputActionSystem::isTextInputActive() const {
    return m_eventManager->isTextInputActive();
}

const std::vector<std::string>& InputActionSystem::getTextInput() const {
    return m_eventManager->getTextInput();
}
void InputActionSystem::getMouseAbsolutePosition(int& x, int& y) const {
    m_eventManager->getMousePosition(x, y);
}
void InputActionSystem::clearTextInputBuffer() {
    m_eventManager->clearTextInputBuffer();
}

void InputActionSystem::handleSpecialKeys(SDL_Keycode keycode) {
    // Gérer les touches spéciales pour l'édition de texte
    switch (keycode) {
        case SDLK_BACKSPACE:
        case SDLK_DELETE:
        case SDLK_RETURN:
        case SDLK_ESCAPE:
        case SDLK_TAB:
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_HOME:
        case SDLK_END:
        case SDLK_PAGEUP:
        case SDLK_PAGEDOWN:
            if (m_specialKeyCallback) {
                m_specialKeyCallback(keycode);
            }
            break;
    }
}