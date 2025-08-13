#include "Loader.h"
#include "SDL_ttf.h"

Loader::Loader() : loadingComplete(false), loadingProgress(0.0f) {}

Loader::~Loader() {
    // S'assurer que le thread de chargement se termine correctement
    if (loadingThread.joinable()) {
        loadingThread.join();
    }
}

void Loader::init() {
    if (TTF_Init() < 0) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        return;
    }
}

void Loader::startLoading(std::function<void(Loader*)> loadingCallback) {
    // Réinitialiser l'état de chargement
    loadingComplete = false;
    loadingProgress = 0.0f;
    
    // Lancer un nouveau thread pour effectuer le chargement
    loadingThread = std::thread([this, loadingCallback]() {
        // Exécuter la fonction de callback de chargement
        loadingCallback(this);
        
        // Marquer le chargement comme terminé
        loadingComplete = true;
    });
}

bool Loader::isLoadingComplete() const {
    return loadingComplete;
}

void Loader::setProgress(float progress) {
    loadingProgress = progress;
}

void Loader::runFrame() {
    // Load the loading screen image
    // Create an empty surface
    SDL_Surface* loadingSurface = SDL_CreateRGBSurface(0, 800, 600, 32, 
                                  0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    
    if (!loadingSurface) {
        std::cerr << "Failed to create loading surface: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Fill the surface with black background
    SDL_FillRect(loadingSurface, NULL, SDL_MapRGB(loadingSurface->format, 0, 0, 0));
    
    TTF_Font* font = TTF_OpenFont("./assets/fonts/AcPlus_IBM_VGA_9x16.ttf", 16);
    if (font) {
        SDL_Color textColor = {255, 255, 255, 255}; // White text
        
        // Afficher le pourcentage de progression
        char loadingText[64];
        snprintf(loadingText, sizeof(loadingText), "Loading... %.0f", (double) this->loadingProgress);
        
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, loadingText, textColor);
        
        if (textSurface) {
            // Calculate position to center the text
            SDL_Rect textRect;
            textRect.x = (loadingSurface->w - textSurface->w) / 2;
            textRect.y = (loadingSurface->h - textSurface->h) / 2;
            
            // Blit the text onto our surface
            SDL_BlitSurface(textSurface, NULL, loadingSurface, &textRect);
            
            // Dessiner une barre de progression
            int barWidth = 400;
            int barHeight = 20;
            SDL_Rect progressBarBg = {
                (loadingSurface->w - barWidth) / 2,
                textRect.y + textSurface->h + 20,
                barWidth,
                barHeight
            };
            
            // Dessiner le contour de la barre
            SDL_FillRect(loadingSurface, &progressBarBg, SDL_MapRGB(loadingSurface->format, 100, 100, 100));
            
            // Dessiner la progression
            SDL_Rect progressBarFill = {
                progressBarBg.x + 2,
                progressBarBg.y + 2,
                static_cast<int>((barWidth - 4) * (this->loadingProgress/100.0f)),
                barHeight - 4
            };
            SDL_FillRect(loadingSurface, &progressBarFill, SDL_MapRGB(loadingSurface->format, 0, 255, 0));
            
            SDL_FreeSurface(textSurface);
        }
        TTF_CloseFont(font);
    }

    // Create OpenGL texture
    GLuint loadingTexture;
    glGenTextures(1, &loadingTexture);
    glBindTexture(GL_TEXTURE_2D, loadingTexture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload the pixel data to the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadingSurface->w, loadingSurface->h, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, loadingSurface->pixels);

    // Free the surface as we don't need it anymore
    SDL_FreeSurface(loadingSurface);

    // Render the loading texture to the screen
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, loadingTexture);

    // Draw a textured quad covering the screen
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDeleteTextures(1, &loadingTexture);
}

void Loader::openScreen() {
    // TV turn-on animation variables
    static bool initialized = false;
    static float animationTime = 0.0f;
    static const float animationDuration = 3.0f;  // 1.5 seconds for the whole animation
    static GLuint scanlineTexture = 0;

    // Initialize the scanline texture the first time
    if (!initialized) {
        // Create scanline texture
        glGenTextures(1, &scanlineTexture);
        glBindTexture(GL_TEXTURE_2D, scanlineTexture);
        
        // Create a 1D pattern for scanlines
        const int texHeight = 256;
        unsigned char* scanlineData = new unsigned char[texHeight * 4];
        for (int i = 0; i < texHeight; i++) {
            float intensity = 0.85f + 0.15f * sin(i * 0.5f);
            scanlineData[i * 4 + 0] = static_cast<unsigned char>(255 * intensity);
            scanlineData[i * 4 + 1] = static_cast<unsigned char>(255 * intensity);
            scanlineData[i * 4 + 2] = static_cast<unsigned char>(255 * intensity);
            scanlineData[i * 4 + 3] = 255;
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, scanlineData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        delete[] scanlineData;
        initialized = true;
    }

    // Update animation time
    const float deltaTime = 1.0f / 60.0f;  // Assuming 60fps
    animationTime = (std::min)(animationTime + deltaTime, animationDuration);
    float animProgress = animationTime / animationDuration;
    if (animProgress >= 1.0f) {
        animProgress = 1.0f;  // Clamp to 1.0
        ison = true;  // Mark that the screen is now on
        return;
    }
    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw TV turning on effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // TV turn-on animation with central point expanding
    if (animProgress < 0.1f) {
        // First phase: Horizontal line expands from center
        float horizontalWidth = animProgress / 0.1f * 2.0f; // 0 to 2.0
        float lineHeight = 0.01f;
        
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
            glVertex2f(-horizontalWidth/2, -lineHeight/2);
            glVertex2f(horizontalWidth/2, -lineHeight/2);
            glVertex2f(horizontalWidth/2, lineHeight/2);
            glVertex2f(-horizontalWidth/2, lineHeight/2);
        glEnd();
    } else if (animProgress < 0.2f) {
        // Second phase: Vertical expansion after horizontal is complete
        float normalizedProgress = (animProgress - 0.1f) / 0.1f; // 0 to 1.0
        float verticalHeight = normalizedProgress * 2.0f;  // 0 to 2.0
        
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
            glVertex2f(-1.0f, -verticalHeight/2);
            glVertex2f(1.0f, -verticalHeight/2);
            glVertex2f(1.0f, verticalHeight/2);
            glVertex2f(-1.0f, verticalHeight/2);
        glEnd();
    } else if (animProgress < 0.3f && animProgress >= 0.5f) {
        // Third phase: Full white screen fading out as noise fades in
        float fadeOutWhite = 1.0f - ((animProgress - 0.5f) / 0.2f);
        
        glColor4f(1.0f, 1.0f, 1.0f, fadeOutWhite);
        glBegin(GL_QUADS);
            glVertex2f(-1.0f, -1.0f);
            glVertex2f(1.0f, -1.0f);
            glVertex2f(1.0f, 1.0f);
            glVertex2f(-1.0f, 1.0f);
        glEnd();
    }
    

    // White noise effect fading out
    if (animProgress >= 0.3f && animProgress < 0.7f) {
        float noiseIntensity = 1.0f - (animProgress / 0.8f);
        
        glColor4f(1.0f, 1.0f, 1.0f, noiseIntensity);
        glBegin(GL_QUADS);
        for (int i = 0; i < 10000; i++) {
            float x = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            float y = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            float size = 0.003f + 0.002f * static_cast<float>(rand()) / RAND_MAX;
            
            glVertex2f(x - size, y - size);
            glVertex2f(x + size, y - size);
            glVertex2f(x + size, y + size);
            glVertex2f(x - size, y + size);
        }
        glEnd();
    }

    // Horizontal line sweeping from center
    if (animProgress > 0.3f && animProgress < 0.8f) {
        float lineProgress = (animProgress - 0.2f) / 0.6f;
        float lineHeight = 0.05f;
        float lineY = (lineProgress * 2.0f - 1.0f);
        
        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, lineY - lineHeight);
        glVertex2f(1.0f, lineY - lineHeight);
        glVertex2f(1.0f, lineY + lineHeight);
        glVertex2f(-1.0f, lineY + lineHeight);
        glEnd();
    }

    // Add scanlines and CRT effect
    if (animProgress > 0.5f) {
        float scanlineAlpha = (animProgress - 0.5f) / 0.5f * 0.3f;
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, scanlineTexture);
        glColor4f(1.0f, 1.0f, 1.0f, scanlineAlpha);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);
        glTexCoord2f(800.0f, 0.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(800.0f, 600.0f); glVertex2f(1.0f, -1.0f);
        glTexCoord2f(0.0f, 600.0f); glVertex2f(-1.0f, -1.0f);
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
    }

    glDisable(GL_BLEND);

}
void Loader::renderMenu() {}

void Loader::renderUI() {}