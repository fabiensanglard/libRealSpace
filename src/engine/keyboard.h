#pragma once

#include <string>
#include "InputActionSystem.h"

class Keyboard {
public:
    Keyboard();
    ~Keyboard();

    
    // Interface pour les actions (abstraction des touches)
    bool isActionPressed(const InputAction actionId) const;
    bool isActionJustPressed(const InputAction actionId) const;
    bool isActionJustReleased(const InputAction actionId) const;
    float getActionValue(const InputAction actionId) const;
    
    bool isKeyJustPressed(SDL_Scancode key) const;

    // Création et configuration des actions
    void registerAction(const InputAction actionId);
    void bindKeyToAction(const InputAction actionId, SDL_Scancode key);
    void bindMouseButtonToAction(const InputAction actionId, int button);
    void bindGamepadButtonToAction(const InputAction actionId, int gamepadIndex, SDL_GameControllerButton button);
    void bindGamepadAxisToAction(const InputAction actionId, int gamepadIndex, SDL_GameControllerAxis axis, float scale = 1.0f);
    void bindMouseAxisToAction(const InputAction actionId, int axis, float scale = 1.0f);
    void bindMousePositionToAction(const InputAction actionId, int axis, float scale = 1.0f);
    void getMouseAbsolutePosition(int& x, int& y) const;
    
    // Persistence des configurations
    bool saveActionBindings(const std::string& filename) const;
    bool loadActionBindings(const std::string& filename);
    
    // Gestion de la zone morte des joysticks
    void setJoystickDeadzone(float deadzone);
    float getJoystickDeadzone() const;
    
    // Mettre à jour l'état des entrées
    void update();
    
    // --- TEXT INPUT SUPPORT ---
    // Mode de saisie de texte
    void startTextInput(const SDL_Rect* inputRect = nullptr);
    void stopTextInput();
    bool isTextInputActive() const;
    
    // Structure pour représenter un éditeur de texte simple
    class TextEditor {
    public:
        TextEditor(Keyboard* keyboard);
        
        // Méthodes pour la manipulation de texte
        void setText(const std::string& text);
        const std::string& getText() const { return m_text; }
        
        // Position du curseur
        void setCursorPosition(int position);
        int getCursorPosition() const { return m_cursorPosition; }
        // Mise à jour (à appeler à chaque frame)
        std::string getDisplayText() const;
        void update();
        
        // Activation/désactivation
        void setActive(bool active, const SDL_Rect* inputRect = nullptr);
        bool isActive() const { return m_active; }
        
    private:
        std::string m_text;
        int m_cursorPosition = 0;
        int m_selectionStart = -1;
        bool m_active = false;
        Keyboard* m_keyboard = nullptr;
        
        void handleTextInput(const char* text);
        void handleSpecialKey(SDL_Keycode key);
    };
    
    // Créer un éditeur de texte
    std::shared_ptr<TextEditor> createTextEditor();
    
private:
    InputActionSystem* m_inputSystem;
};