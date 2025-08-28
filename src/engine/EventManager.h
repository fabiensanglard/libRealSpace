#pragma once

#include <SDL.h>
#include <SDL_gamecontroller.h>
#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>
#include <string>

class EventManager {
public:
    // Singleton pattern
    static EventManager& getInstance();
    
    // Suppression des constructeurs de copie/déplacement
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;
    EventManager(EventManager&&) = delete;
    EventManager& operator=(EventManager&&) = delete;

    // Initialisation et fermeture
    void initialize();
    void shutdown();

    // Met à jour l'état des événements (à appeler une fois par frame)
    void update();
    
    // Vérifie si l'application doit se terminer
    bool shouldQuit() const { return m_quit; }
    
    // État du clavier
    bool isKeyPressed(SDL_Scancode key) const;
    bool isKeyJustPressed(SDL_Scancode key) const;
    bool isKeyJustReleased(SDL_Scancode key) const;
    
    // État de la souris
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonJustPressed(int button) const;
    bool isMouseButtonJustReleased(int button) const;
    void getMousePosition(int& x, int& y) const;
    void getMouseMotion(int& dx, int& dy) const;
    
    // Gestionnaires d'événements personnalisés
    using EventCallback = std::function<void(const SDL_Event&)>;
    int addEventCallback(SDL_EventType type, EventCallback callback);
    void removeEventCallback(SDL_EventType type, int callbackId);

    // --- JOYSTICK & GAMEPAD SUPPORT ---
    // Nombre de joysticks/gamepads connectés
    int getJoystickCount() const;
    
    // Vérifier si un joystick est un gamepad
    bool isJoystickGamepad(int joystickIndex) const;
    
    // Obtenir le nom d'un joystick/gamepad
    std::string getJoystickName(int joystickIndex) const;
    
    // État des boutons du joystick
    bool isJoystickButtonPressed(int joystickIndex, int button) const;
    bool isJoystickButtonJustPressed(int joystickIndex, int button) const;
    bool isJoystickButtonJustReleased(int joystickIndex, int button) const;
    
    // Valeur des axes du joystick (entre -1.0 et 1.0)
    float getJoystickAxisValue(int joystickIndex, int axis) const;
    
    // État des boutons gamepad (avec mappage standard)
    bool isGamepadButtonPressed(int gamepadIndex, SDL_GameControllerButton button) const;
    bool isGamepadButtonJustPressed(int gamepadIndex, SDL_GameControllerButton button) const;
    bool isGamepadButtonJustReleased(int gamepadIndex, SDL_GameControllerButton button) const;
    
    // Valeur des axes gamepad (entre -1.0 et 1.0)
    float getGamepadAxisValue(int gamepadIndex, SDL_GameControllerAxis axis) const;
    
    // Zone morte pour les joysticks (valeur en dessous de laquelle l'entrée est ignorée)
    void setJoystickDeadzone(float deadzone) { m_joystickDeadzone = deadzone; }
    float getJoystickDeadzone() const { return m_joystickDeadzone; }

    // --- TEXT INPUT SUPPORT ---
    // Active/désactive le mode de saisie de texte
    void startTextInput(const SDL_Rect* inputRect = nullptr);
    void stopTextInput();
    bool isTextInputActive() const;
    
    // Définit une zone de l'écran pour l'IME (composition de texte)
    void setTextInputRect(const SDL_Rect& rect);
    
    // Callbacks pour la saisie de texte
    using TextInputCallback = std::function<void(const char*)>;
    using TextEditingCallback = std::function<void(const char*, int, int)>;
    using KeyPressCallback = std::function<void(SDL_Keycode, Uint16)>;
    
    void setTextInputCallback(TextInputCallback callback);
    void setTextEditingCallback(TextEditingCallback callback);
    void setKeyPressCallback(KeyPressCallback callback);
    
    // Récupérer le texte saisi durant cette frame
    const std::vector<std::string>& getTextInput() const { return m_textInputBuffer; }
    
    // Effacer le buffer de texte (à appeler après avoir traité le texte)
    void clearTextInputBuffer() { m_textInputBuffer.clear(); }

private:
    EventManager();
    ~EventManager();
    
    // Gestion des joysticks/gamepads
    void openJoysticks();
    void closeJoysticks();
    void handleJoystickEvent(const SDL_Event& event);
    
    // Gestion des événements de texte
    void handleTextInputEvent(const SDL_Event& event);
    
    bool m_quit = false;
    
    // État actuel et précédent du clavier
    const Uint8* m_currentKeyStates = nullptr;
    std::vector<Uint8> m_previousKeyStates;
    int m_numKeys = 0;
    
    // État de la souris
    struct {
        Uint32 currentButtonState = 0;
        Uint32 previousButtonState = 0;
        int x = 0, y = 0;
        int dx = 0, dy = 0;
    } m_mouse;
    
    // Callbacks d'événements personnalisés
    struct CallbackData {
        EventCallback callback;
        int id;
    };
    
    std::unordered_map<SDL_EventType, std::vector<CallbackData>> m_eventCallbacks;
    int m_nextCallbackId = 0;
    
    // Structure pour stocker l'état des joysticks
    struct JoystickState {
        SDL_Joystick* joystick = nullptr;
        SDL_GameController* gameController = nullptr;
        SDL_JoystickID instanceId = -1;
        std::vector<Uint8> currentButtonStates;
        std::vector<Uint8> previousButtonStates;
        std::vector<float> axisValues;
        bool isGamepad = false;
    };
    
    std::vector<JoystickState> m_joysticks;
    std::unordered_map<SDL_JoystickID, int> m_joystickInstanceMap;
    float m_joystickDeadzone = 0.1f;
    
    // Support de saisie de texte
    bool m_textInputActive = false;
    std::vector<std::string> m_textInputBuffer;  // Texte saisi durant cette frame
    TextInputCallback m_textInputCallback = nullptr;
    TextEditingCallback m_textEditingCallback = nullptr;
    KeyPressCallback m_keyPressCallback = nullptr;
};