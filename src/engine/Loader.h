#pragma once
#include <SDL.h>
#ifdef _WIN32
#include <Windows.h>
#include <GL/GL.h>
#endif
#include <SDL_opengl_glext.h>
#include <SDL_opengl.h>
#include <iostream>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

class Loader {
public:
    Loader();
    ~Loader();

    void init();
    
    // Fonction pour démarrer le chargement en arrière-plan
    void startLoading(std::function<void(Loader*)> loadingCallback);
    
    // Affiche un frame de l'écran de chargement
    void runFrame();
    
    // Vérifie si le chargement est terminé
    bool isLoadingComplete() const;
    
    // Fonction pour mettre à jour la progression du chargement
    void setProgress(float progress);

private:
    std::thread loadingThread;
    std::atomic<bool> loadingComplete;
    std::atomic<float> loadingProgress;
    std::mutex progressMutex;
    
};