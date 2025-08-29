#pragma once

#include <SDL.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

enum class InputAction {
// Actions de base du système (0-999)
    UNKNOWN = 0,
    MOUSE_LEFT = 1,
    MOUSE_RIGHT = 2,
    MOUSE_MIDDLE = 3,
    MOUSE_POS_X = 4,
    MOUSE_POS_Y = 5,
    MOUSE_WHEEL_UP = 6,
    MOUSE_WHEEL_DOWN = 7,
    KEY_ESCAPE = 8,
    KEY_ENTER = 9,
    

    // Plage réservée pour le File Requester (1000-1999)
    FILE_REQUESTER_START = 1000,
    FILE_REQUESTER_CANCEL = 1001,


    FILE_REQUESTER_END = 1999,
    
    // Plage réservée pour le simulateur de vol (2000-2999)
    SIM_START = 2000,
    // Aucune action prédéfinie ici, elles seront définies dynamiquement
    SIM_END = 2999,
    
    
    // Valeurs spéciales
    CUSTOM_ACTION_START = 10000  // Début de la plage pour actions personnalisées
};

// Fonction utilitaire pour créer des actions dynamiques dans une plage réservée
inline InputAction CreateAction(InputAction baseAction, int offset) {
    return static_cast<InputAction>(static_cast<int>(baseAction) + offset);
}

// Type d'entrée
enum class InputType {
    KEYBOARD,
    MOUSE_BUTTON,
    MOUSE_AXIS,
    MOUSE_POSITION,
    GAMEPAD_BUTTON,
    GAMEPAD_AXIS
};

// Structure pour représenter une entrée spécifique
struct InputBinding {
    InputType type;
    int deviceId;  // Pour distinguer plusieurs souris/manettes (-1 = périphérique par défaut)
    int code;      // Code spécifique à l'entrée (touche, bouton, axe...)
    float scale;   // Pour les axes (1.0 ou -1.0 pour inverser)
    
    InputBinding(InputType t, int dev, int c, float s = 1.0f)
        : type(t), deviceId(dev), code(c), scale(s) {}
};

// Classe principale du système d'actions
class InputActionSystem {
public:
    enum class InputMode {
        NORMAL,     // Mode normal : traitement des actions mappées
        TEXT_INPUT  // Mode saisie de texte : capture directe du texte
    };
    void setInputMode(InputMode mode);
    InputMode getInputMode() const { return m_inputMode; }
    
    // Fonctions de saisie de texte
    void startTextInput(const SDL_Rect* inputRect = nullptr);
    void stopTextInput();
    bool isTextInputActive() const;
    
    // Callbacks pour la saisie de texte
    using TextInputCallback = std::function<void(const char*)>;
    using TextEditingCallback = std::function<void(const char*, int, int)>;
    using SpecialKeyCallback = std::function<void(SDL_Keycode)>;
    
    void setTextInputCallback(TextInputCallback callback) { m_textInputCallback = callback; }
    void setTextEditingCallback(TextEditingCallback callback) { m_textEditingCallback = callback; }
    void setSpecialKeyCallback(SpecialKeyCallback callback) { m_specialKeyCallback = callback; }
    
    // Récupérer le texte saisi durant cette frame
    const std::vector<std::string>& getTextInput() const;
    
    // Effacer le buffer de texte
    void clearTextInputBuffer();
    static InputActionSystem& getInstance();
    
    // Empêche la copie/déplacement
    InputActionSystem(const InputActionSystem&) = delete;
    InputActionSystem& operator=(const InputActionSystem&) = delete;
    InputActionSystem(InputActionSystem&&) = delete;
    InputActionSystem& operator=(InputActionSystem&&) = delete;
    
    // Initialisation/fermeture
    void initialize();
    void shutdown();
    
    // Mise à jour de l'état des entrées
    void update();
    
    // Créer une action avec un identifiant unique
    void registerAction(const InputAction actionId);
    
    // Associer une entrée à une action
    void bindInput(const InputAction actionId, const InputBinding& binding);
    
    // Supprimer une liaison pour une action
    void unbindInput(const InputAction actionId, const InputBinding& binding);
    
    // Supprimer toutes les liaisons pour une action
    void clearBindings(const InputAction actionId);
    
    // Interroger l'état d'une action
    bool isActionPressed(const InputAction actionId) const;
    bool isActionJustPressed(const InputAction actionId) const;
    bool isActionJustReleased(const InputAction actionId) const;
    float getActionValue(const InputAction actionId) const;  // Pour les axes (-1.0 à 1.0)
    void getMouseAbsolutePosition(int& x, int& y) const;
    
    // Sauvegarder/charger des configurations
    bool saveBindings(const std::string &filename) const;
    bool loadBindings(const std::string &filename);
    
private:
    InputActionSystem();
    ~InputActionSystem();
    
    // Structure pour stocker l'état d'une action
    struct ActionState {
        bool currentState = false;
        bool previousState = false;
        float currentValue = 0.0f;
        float previousValue = 0.0f;
        std::vector<InputBinding> bindings;
    };
    
    std::unordered_map<InputAction, ActionState> m_actions;
    float m_axisDeadZone = 0.1f;
    InputMode m_inputMode = InputMode::NORMAL;
    
    // Callbacks pour la saisie de texte
    TextInputCallback m_textInputCallback = nullptr;
    TextEditingCallback m_textEditingCallback = nullptr;
    SpecialKeyCallback m_specialKeyCallback = nullptr;
    
    // Traitement des touches spéciales en mode texte
    void handleSpecialKeys(SDL_Keycode keycode);
    // Référence à l'EventManager
    class EventManager* m_eventManager = nullptr;
    
    bool evaluateBinding(const InputBinding& binding, float& outValue) const;
};
