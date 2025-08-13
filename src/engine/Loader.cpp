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

void Loader::renderMenu() {}

void Loader::renderUI() {}