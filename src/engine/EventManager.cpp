#include "EventManager.h"
#include <SDL_gamecontroller.h>
#include <cstring>
#include <algorithm>


EventManager& EventManager::getInstance() {
    static EventManager instance;
    return instance;
}

EventManager::EventManager() {
    // Le constructeur est maintenant minimal, l'initialisation se fait dans initialize()
}

EventManager::~EventManager() {
    shutdown();
}

void EventManager::initialize() {
    // Initialiser le sous-système joystick et gamecontroller de SDL
    SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
    
    // Activer les événements de joystick
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_GameControllerEventState(SDL_ENABLE);
    
    m_currentKeyStates = SDL_GetKeyboardState(&m_numKeys);
    m_previousKeyStates.resize(m_numKeys);
    
    // Copier l'état initial du clavier
    std::memcpy(m_previousKeyStates.data(), m_currentKeyStates, m_numKeys);
    
    // Détecter et ouvrir les joysticks connectés
    openJoysticks();
}

void EventManager::shutdown() {
    // Fermer tous les joysticks et gamepads
    closeJoysticks();
}
void EventManager::startTextInput(const SDL_Rect* inputRect) {
    if (!m_textInputActive) {
        m_textInputActive = true;
        SDL_StartTextInput();
        
        if (inputRect) {
            setTextInputRect(*inputRect);
        }
    }
}

void EventManager::stopTextInput() {
    if (m_textInputActive) {
        m_textInputActive = false;
        SDL_StopTextInput();
        m_textInputBuffer.clear();
    }
}

bool EventManager::isTextInputActive() const {
    return m_textInputActive;
}

void EventManager::setTextInputRect(const SDL_Rect& rect) {
    SDL_SetTextInputRect(&rect);
}

void EventManager::setTextInputCallback(TextInputCallback callback) {
    m_textInputCallback = callback;
}

void EventManager::setTextEditingCallback(TextEditingCallback callback) {
    m_textEditingCallback = callback;
}

void EventManager::setKeyPressCallback(KeyPressCallback callback) {
    m_keyPressCallback = callback;
}

void EventManager::handleTextInputEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_TEXTINPUT:
            // Ajouter le texte saisi au buffer
            m_textInputBuffer.push_back(event.text.text);
            
            // Appeler le callback s'il existe
            if (m_textInputCallback) {
                m_textInputCallback(event.text.text);
            }
            break;
            
        case SDL_TEXTEDITING:
            // Appeler le callback de composition de texte s'il existe
            if (m_textEditingCallback) {
                m_textEditingCallback(event.edit.text, event.edit.start, event.edit.length);
            }
            break;
            
        case SDL_KEYDOWN:
            // Appeler le callback de touche pressée s'il existe
            if (m_keyPressCallback && m_textInputActive) {
                m_keyPressCallback(event.key.keysym.sym, event.key.keysym.mod);
            }
            break;
    }
}
void EventManager::openJoysticks() {
    closeJoysticks();  // Fermer d'abord les joysticks existants
    
    int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; ++i) {
        JoystickState state;
        
        // Vérifier si c'est un gamepad
        state.isGamepad = SDL_IsGameController(i);
        
        if (state.isGamepad) {
            // Ouvrir comme un gamecontroller
            state.gameController = SDL_GameControllerOpen(i);
            if (state.gameController) {
                state.joystick = SDL_GameControllerGetJoystick(state.gameController);
                state.instanceId = SDL_JoystickInstanceID(state.joystick);
                
                // Allouer des tableaux pour les boutons
                int numButtons = SDL_CONTROLLER_BUTTON_MAX;
                state.currentButtonStates.resize(numButtons, 0);
                state.previousButtonStates.resize(numButtons, 0);
                
                // Allouer tableau pour les axes
                state.axisValues.resize(SDL_CONTROLLER_AXIS_MAX, 0.0f);
                
                m_joysticks.push_back(state);
                m_joystickInstanceMap[state.instanceId] = static_cast<int>(m_joysticks.size() - 1);
            }
        } else {
            // Ouvrir comme un joystick standard
            state.joystick = SDL_JoystickOpen(i);
            if (state.joystick) {
                state.instanceId = SDL_JoystickInstanceID(state.joystick);
                
                // Allouer des tableaux pour les boutons
                int numButtons = SDL_JoystickNumButtons(state.joystick);
                state.currentButtonStates.resize(numButtons, 0);
                state.previousButtonStates.resize(numButtons, 0);
                
                // Allouer tableau pour les axes
                int numAxes = SDL_JoystickNumAxes(state.joystick);
                state.axisValues.resize(numAxes, 0.0f);
                
                m_joysticks.push_back(state);
                m_joystickInstanceMap[state.instanceId] = static_cast<int>(m_joysticks.size() - 1);
            }
        }
    }
}

void EventManager::closeJoysticks() {
    for (auto& joystick : m_joysticks) {
        if (joystick.gameController) {
            SDL_GameControllerClose(joystick.gameController);
        } else if (joystick.joystick) {
            SDL_JoystickClose(joystick.joystick);
        }
    }
    
    m_joysticks.clear();
    m_joystickInstanceMap.clear();
}

void EventManager::update() {
    // Sauvegarder l'état précédent du clavier
    std::memcpy(m_previousKeyStates.data(), m_currentKeyStates, m_numKeys);
    
    // Sauvegarder l'état précédent de la souris
    m_mouse.previousButtonState = m_mouse.currentButtonState;
    m_mouse.dx = 0;
    m_mouse.dy = 0;
    
    // Sauvegarder l'état précédent des joysticks
    for (auto& joystick : m_joysticks) {
        std::copy(joystick.currentButtonStates.begin(), joystick.currentButtonStates.end(),
                  joystick.previousButtonStates.begin());
    }
    m_textInputBuffer.clear();
    // Traiter tous les événements en attente
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                m_quit = true;
                break;
                
            case SDL_MOUSEMOTION:
                m_mouse.x = event.motion.x;
                m_mouse.y = event.motion.y;
                m_mouse.dx += event.motion.xrel;
                m_mouse.dy += event.motion.yrel;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                m_mouse.x = event.button.x;
                m_mouse.y = event.button.y;
                if (event.button.state == SDL_PRESSED) {
                    m_mouse.currentButtonState |= (1 << event.button.button);
                } else {
                    m_mouse.currentButtonState &= ~(1 << event.button.button);
                }
                break;
                
            // Événements de joystick
            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
                // Réinitialiser la liste des joysticks
                openJoysticks();
                break;
                
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            case SDL_JOYAXISMOTION:
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERAXISMOTION:
                handleJoystickEvent(event);
                break;
            case SDL_TEXTINPUT:
            case SDL_TEXTEDITING:
                handleTextInputEvent(event);
                break;
                
            case SDL_KEYDOWN:
                handleTextInputEvent(event);
                // Gérer le keydown pour d'autres fonctionnalités...
                break;
        }
        
        // Appeler les callbacks enregistrés pour ce type d'événement
        auto callbacksIt = m_eventCallbacks.find(static_cast<SDL_EventType>(event.type));
        if (callbacksIt != m_eventCallbacks.end()) {
            for (const auto& callbackData : callbacksIt->second) {
                callbackData.callback(event);
            }
        }
    }
    
    // L'état du clavier est mis à jour automatiquement par SDL_PumpEvents() appelé par SDL_PollEvent()
}

void EventManager::handleJoystickEvent(const SDL_Event& event) {
    SDL_JoystickID instanceId = -1;
    int buttonIndex = -1;
    int axisIndex = -1;
    Uint8 buttonState = 0;
    float axisValue = 0.0f;
    
    // Extraire l'ID d'instance et les données pertinentes selon le type d'événement
    switch (event.type) {
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            instanceId = event.jbutton.which;
            buttonIndex = event.jbutton.button;
            buttonState = event.jbutton.state;
            break;
            
        case SDL_JOYAXISMOTION:
            instanceId = event.jaxis.which;
            axisIndex = event.jaxis.axis;
            // Normaliser la valeur de l'axe entre -1.0 et 1.0
            axisValue = static_cast<float>(event.jaxis.value) / 32768.0f;
            break;
            
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            instanceId = event.cbutton.which;
            buttonIndex = event.cbutton.button;
            buttonState = event.cbutton.state;
            break;
            
        case SDL_CONTROLLERAXISMOTION:
            instanceId = event.caxis.which;
            axisIndex = event.caxis.axis;
            // Normaliser la valeur de l'axe entre -1.0 et 1.0
            axisValue = static_cast<float>(event.caxis.value) / 32768.0f;
            break;
    }
    
    // Trouver le joystick correspondant à l'ID d'instance
    auto it = m_joystickInstanceMap.find(instanceId);
    if (it != m_joystickInstanceMap.end()) {
        int joystickIndex = it->second;
        
        // Mettre à jour l'état du bouton
        if (buttonIndex >= 0 && static_cast<size_t>(buttonIndex) < m_joysticks[joystickIndex].currentButtonStates.size()) {
            m_joysticks[joystickIndex].currentButtonStates[buttonIndex] = buttonState;
        }
        
        // Mettre à jour la valeur de l'axe
        if (axisIndex >= 0 && static_cast<size_t>(axisIndex) < m_joysticks[joystickIndex].axisValues.size()) {
            // Appliquer une zone morte
            if (std::abs(axisValue) < m_joystickDeadzone) {
                axisValue = 0.0f;
            } else {
                // Normaliser la valeur après application de la zone morte
                float sign = (axisValue > 0) ? 1.0f : -1.0f;
                axisValue = sign * (std::abs(axisValue) - m_joystickDeadzone) / (1.0f - m_joystickDeadzone);
            }
            
            m_joysticks[joystickIndex].axisValues[axisIndex] = axisValue;
        }
    }
}

bool EventManager::isKeyPressed(SDL_Scancode key) const {
    return m_currentKeyStates[key] != 0;
}

bool EventManager::isKeyJustPressed(SDL_Scancode key) const {
    return m_currentKeyStates[key] && !m_previousKeyStates[key];
}

bool EventManager::isKeyJustReleased(SDL_Scancode key) const {
    return !m_currentKeyStates[key] && m_previousKeyStates[key];
}

bool EventManager::isMouseButtonPressed(int button) const {
    return (m_mouse.currentButtonState & (1 << button)) != 0;
}

bool EventManager::isMouseButtonJustPressed(int button) const {
    const Uint32 mask = 1 << button;
    return (m_mouse.currentButtonState & mask) && !(m_mouse.previousButtonState & mask);
}

bool EventManager::isMouseButtonJustReleased(int button) const {
    const Uint32 mask = 1 << button;
    return !(m_mouse.currentButtonState & mask) && (m_mouse.previousButtonState & mask);
}

void EventManager::getMousePosition(int& x, int& y) const {
    x = m_mouse.x;
    y = m_mouse.y;
}

void EventManager::getMouseMotion(int& dx, int& dy) const {
    dx = m_mouse.dx;
    dy = m_mouse.dy;
}

int EventManager::addEventCallback(SDL_EventType type, EventCallback callback) {
    int callbackId = m_nextCallbackId++;
    m_eventCallbacks[type].push_back({callback, callbackId});
    return callbackId;
}

void EventManager::removeEventCallback(SDL_EventType type, int callbackId) {
    auto& callbacks = m_eventCallbacks[type];
    callbacks.erase(
        std::remove_if(callbacks.begin(), callbacks.end(),
                      [callbackId](const CallbackData& data) { return data.id == callbackId; }),
        callbacks.end());
}

// --- Méthodes pour les joysticks/gamepads ---

int EventManager::getJoystickCount() const {
    return static_cast<int>(m_joysticks.size());
}

bool EventManager::isJoystickGamepad(int joystickIndex) const {
    if (joystickIndex >= 0 && joystickIndex < static_cast<int>(m_joysticks.size())) {
        return m_joysticks[joystickIndex].isGamepad;
    }
    return false;
}

std::string EventManager::getJoystickName(int joystickIndex) const {
    if (joystickIndex >= 0 && joystickIndex < static_cast<int>(m_joysticks.size())) {
        const auto& joystick = m_joysticks[joystickIndex];
        if (joystick.isGamepad && joystick.gameController) {
            return SDL_GameControllerName(joystick.gameController);
        } else if (joystick.joystick) {
            return SDL_JoystickName(joystick.joystick);
        }
    }
    return "Unknown";
}

bool EventManager::isJoystickButtonPressed(int joystickIndex, int button) const {
    if (joystickIndex >= 0 && joystickIndex < static_cast<int>(m_joysticks.size())) {
        const auto& joystick = m_joysticks[joystickIndex];
        if (button >= 0 && button < static_cast<int>(joystick.currentButtonStates.size())) {
            return joystick.currentButtonStates[button] != 0;
        }
    }
    return false;
}

bool EventManager::isJoystickButtonJustPressed(int joystickIndex, int button) const {
    if (joystickIndex >= 0 && joystickIndex < static_cast<int>(m_joysticks.size())) {
        const auto& joystick = m_joysticks[joystickIndex];
        if (button >= 0 && button < static_cast<int>(joystick.currentButtonStates.size())) {
            return joystick.currentButtonStates[button] && !joystick.previousButtonStates[button];
        }
    }
    return false;
}

bool EventManager::isJoystickButtonJustReleased(int joystickIndex, int button) const {
    if (joystickIndex >= 0 && joystickIndex < static_cast<int>(m_joysticks.size())) {
        const auto& joystick = m_joysticks[joystickIndex];
        if (button >= 0 && button < static_cast<int>(joystick.currentButtonStates.size())) {
            return !joystick.currentButtonStates[button] && joystick.previousButtonStates[button];
        }
    }
    return false;
}

float EventManager::getJoystickAxisValue(int joystickIndex, int axis) const {
    if (joystickIndex >= 0 && joystickIndex < static_cast<int>(m_joysticks.size())) {
        const auto& joystick = m_joysticks[joystickIndex];
        if (axis >= 0 && axis < static_cast<int>(joystick.axisValues.size())) {
            return joystick.axisValues[axis];
        }
    }
    return 0.0f;
}

bool EventManager::isGamepadButtonPressed(int gamepadIndex, SDL_GameControllerButton button) const {
    int count = 0;
    for (size_t i = 0; i < m_joysticks.size(); ++i) {
        if (m_joysticks[i].isGamepad) {
            if (count == gamepadIndex) {
                return isJoystickButtonPressed(i, static_cast<int>(button));
            }
            count++;
        }
    }
    return false;
}

bool EventManager::isGamepadButtonJustPressed(int gamepadIndex, SDL_GameControllerButton button) const {
    int count = 0;
    for (size_t i = 0; i < m_joysticks.size(); ++i) {
        if (m_joysticks[i].isGamepad) {
            if (count == gamepadIndex) {
                return isJoystickButtonJustPressed(i, static_cast<int>(button));
            }
            count++;
        }
    }
    return false;
}

bool EventManager::isGamepadButtonJustReleased(int gamepadIndex, SDL_GameControllerButton button) const {
    int count = 0;
    for (size_t i = 0; i < m_joysticks.size(); ++i) {
        if (m_joysticks[i].isGamepad) {
            if (count == gamepadIndex) {
                return isJoystickButtonJustReleased(i, static_cast<int>(button));
            }
            count++;
        }
    }
    return false;
}

float EventManager::getGamepadAxisValue(int gamepadIndex, SDL_GameControllerAxis axis) const {
    int count = 0;
    for (size_t i = 0; i < m_joysticks.size(); ++i) {
        if (m_joysticks[i].isGamepad) {
            if (count == gamepadIndex) {
                return getJoystickAxisValue(i, static_cast<int>(axis));
            }
            count++;
        }
    }
    return 0.0f;
}