#include "../../engine/Config.hpp"
#include "../debugger/DebugControlMapping.h"
#include "../../engine/EventManager.h"  
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <string>

static std::string _labelPrefix(const char* const label)
{
	float width = ImGui::CalcItemWidth();

	float x = ImGui::GetCursorPosX();
	ImGui::Text(label); 
	ImGui::SameLine(); 
	ImGui::SetCursorPosX(x + width * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::SetNextItemWidth(-1);

	std::string labelID = "##";
	labelID += label;

	return labelID;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow("Config Editor",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        500, 550, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    const std::string configPath = "./assets/config.ini";
    Config& config = Config::getInstance();
    config.load(configPath);

    // [Window]
    int win_width  = config.getInt("Window", "width", 1920);
    int win_height = config.getInt("Window", "height", 1080);
    bool fullscreen = config.getBool("Window", "fullscreen", false);

    // [Game]
    int object_detail = config.getInt("Game", "object_detail", 0);
    int world_detail  = config.getInt("Game", "world_detail", 0);
    int max_view_distance = config.getInt("Game", "max_view_distance", 300000);
    bool show_texture = config.getBool("Game", "show_texture", true);
    bool show_fog     = config.getBool("Game", "show_fog", true);
    bool clouds_enabled = config.getBool("Game", "clouds_enabled", false);
    // [Video]
    bool fx_cpc_palette  = config.getBool("Video", "fx_cpc_palette", false);
    bool fx_scanlines    = config.getBool("Video", "fx_scanlines", false);
    int  fx_pixel_scale  = config.getInt ("Video", "fx_pixel_scale", 1);
    bool super_eagle_2x  = config.getBool("Video", "super_eagle_2x", true);
    bool fx_fxaa         = config.getBool("Video", "fx_fxaa", true);
    bool widescreen_ambilight = config.getBool("Video", "widescreen_ambilight", true);
    int ambilight_sample_width = config.getInt("Video", "ambilight_sample_width", 1);
    int terrain_shadow_lift = config.getInt("Game", "terrain_shadow_lift", 8);
    int terrain_shadow_range = config.getInt("Game", "terrain_shadow_range", 35);
    bool terrain_flat = config.getBool("Game", "terrain_flat", false);
    int difficulty = config.getInt("Gameplay", "difficulty", 0);
    // [Input]
    char bindings_file[256];
    snprintf(bindings_file, sizeof(bindings_file), "%s", config.getString("Input", "bindings_file", "default_bindings.cfg").c_str());
    // [Sound]
    char midi_port[256];
    snprintf(midi_port, sizeof(midi_port), "%s", config.getString("Sound", "midi_port", "").c_str());
    int midi_music = config.getInt("Sound", "midi_music", 0);
    bool midi_debug = config.getInt("Sound", "midi_debug", 0) != 0;
    char soundfont[512];
    snprintf(soundfont, sizeof(soundfont), "%s", config.getString("Sound", "soundfont", "").c_str());
    const char *midi_music_labels[] = {
        "AdLib",
        "General MIDI (SC-155)",
        "Roland MT-32",
    };
    DebugControlMapping controlMapping;
    GameEngine::setInstance(std::make_unique<GameEngine>());
    GameEngine *game = &GameEngine::instance();
    EventManager::getInstance().enableImGuiForwarding(true);
    AssetManager::getInstance().SetBase("./assets/");
    game->init();
    controlMapping.init();
    bool running = true;
    struct Resolution { int w, h, refresh; };
    std::vector<Resolution> resolutions;
    int numModes = SDL_GetNumDisplayModes(0);
    for (int i = 0; i < numModes; i++) {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(0, i, &mode) == 0) {
            // Dédoublonner (même résolution, taux de rafraîchissement différent)
            bool found = false;
            for (auto& r : resolutions)
                if (r.w == mode.w && r.h == mode.h) { found = true; break; }
            if (!found)
                resolutions.push_back({mode.w, mode.h, mode.refresh_rate});
        }
    }
    // Trouver l'index courant
    int selectedRes = 0;
    for (int i = 0; i < (int)resolutions.size(); i++) {
        if (resolutions[i].w == win_width && resolutions[i].h == win_height) {
            selectedRes = i;
            break;
        }
    }
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) running = false;
        }

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        ImGui::Begin("Configuration", nullptr,
                    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

        if (ImGui::BeginTabBar("tabs")) {
            if (ImGui::BeginTabItem("General")) {
                if (ImGui::CollapsingHeader("Window", ImGuiTreeNodeFlags_DefaultOpen)) {
                    // Construire les labels à la volée
                    if (ImGui::BeginCombo(_labelPrefix("Resolution").c_str(), 
                        (std::to_string(resolutions[selectedRes].w) + "x" +
                        std::to_string(resolutions[selectedRes].h)).c_str()))
                    {
                        for (int i = 0; i < (int)resolutions.size(); i++) {
                            std::string label = std::to_string(resolutions[i].w) + "x" +
                                                std::to_string(resolutions[i].h);
                            bool isSelected = (i == selectedRes);
                            if (ImGui::Selectable(label.c_str(), isSelected)) {
                                selectedRes  = i;
                                win_width    = resolutions[i].w;
                                win_height   = resolutions[i].h;
                            }
                            if (isSelected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::Checkbox(_labelPrefix("Fullscreen").c_str(), &fullscreen);
                }
                if (ImGui::CollapsingHeader("Game", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::SliderInt(_labelPrefix("Object Detail").c_str(), &object_detail, 0, 2);
                    ImGui::SliderInt(_labelPrefix("World Detail").c_str(),  &world_detail,  0, 2);
                    ImGui::SliderInt(_labelPrefix("Max view distance").c_str(), &max_view_distance, 10000, 300000);
                    ImGui::Checkbox(_labelPrefix("Show Texture").c_str(), &show_texture);
                    ImGui::Checkbox(_labelPrefix("Show Fog").c_str(), &show_fog);
                    ImGui::Checkbox(_labelPrefix("Clouds Enabled").c_str(), &clouds_enabled);
                    ImGui::SliderInt(_labelPrefix("Terrain Shadow Lift").c_str(), &terrain_shadow_lift, 0, 50);
                    ImGui::SliderInt(_labelPrefix("Terrain Shadow Range").c_str(), &terrain_shadow_range, 1, 100);
                    ImGui::Checkbox(_labelPrefix("Terrain Flat Shading").c_str(), &terrain_flat);
                }
                if (ImGui::CollapsingHeader("Gameplay", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::SliderInt(_labelPrefix("Difficulty").c_str(), &difficulty, 0, 2);
                }
                if (ImGui::CollapsingHeader("Video", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Checkbox(_labelPrefix("CPC Palette").c_str(),   &fx_cpc_palette);
                    ImGui::Checkbox(_labelPrefix("Scanlines").c_str(),     &fx_scanlines);
                    ImGui::SliderInt(_labelPrefix("Pixel Scale").c_str(),   &fx_pixel_scale, 1, 4);
                    ImGui::Checkbox(_labelPrefix("FXAA").c_str(),           &fx_fxaa);
                    ImGui::Checkbox(_labelPrefix("Super Eagle 2x").c_str(),&super_eagle_2x);
                    ImGui::Checkbox(_labelPrefix("Widescreen Ambilight").c_str(), &widescreen_ambilight);
                    ImGui::SliderInt(_labelPrefix("Ambilight Sample Width").c_str(), &ambilight_sample_width, 1, 10);

                }
                if (ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::InputText(_labelPrefix("Bindings File").c_str(), bindings_file, sizeof(bindings_file));
                }
                if (ImGui::CollapsingHeader("Sound", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Combo(_labelPrefix("Music Output").c_str(), &midi_music, midi_music_labels, 3);
                    ImGui::InputText(_labelPrefix("MIDI Port").c_str(), midi_port, sizeof(midi_port));
                    ImGui::InputText(_labelPrefix("SoundFont").c_str(), soundfont, sizeof(soundfont));
                    ImGui::Checkbox(_labelPrefix("MIDI Debug").c_str(), &midi_debug);
                }

                ImGui::Separator();
                if (ImGui::Button("Save")) {
                    config.setInt ("Window", "width",       win_width);
                    config.setInt ("Window", "height",      win_height);
                    config.setBool("Window", "fullscreen",  fullscreen);
                    config.setInt ("Game",   "object_detail", object_detail);
                    config.setInt ("Game",   "world_detail",  world_detail);
                    config.setInt ("Game",   "max_view_distance", max_view_distance);
                    config.setBool("Game",   "show_texture",  show_texture);
                    config.setBool("Game",   "show_fog",      show_fog);
                    config.setBool("Game",   "clouds_enabled", clouds_enabled);
                    config.setBool("Video",  "fx_cpc_palette", fx_cpc_palette);
                    config.setBool("Video",  "fx_scanlines",   fx_scanlines);
                    config.setInt ("Video",  "fx_pixel_scale",  fx_pixel_scale);
                    config.setBool("Video",  "super_eagle_2x",  super_eagle_2x);
                    config.setBool("Video",  "fx_fxaa",          fx_fxaa);
                    config.setBool("Video",  "widescreen_ambilight", widescreen_ambilight);
                    config.setInt ("Video",  "ambilight_sample_width", ambilight_sample_width);
                    config.setInt ("Game",   "terrain_shadow_lift", terrain_shadow_lift);
                    config.setInt ("Game",   "terrain_shadow_range", terrain_shadow_range);
                    config.setBool("Game",   "terrain_flat", terrain_flat);
                    config.setInt ("Gameplay", "difficulty", difficulty);
                    config.setString("Input", "bindings_file", bindings_file);
                    config.setInt ("Sound",  "midi_music", midi_music);
                    config.setString("Sound", "midi_port", midi_port);
                    config.setString("Sound", "soundfont", soundfont);
                    config.setInt ("Sound",  "midi_debug", midi_debug ? 1 : 0);
                    config.save(configPath);
                }
                ImGui::SameLine();
                if (ImGui::Button("Quit")) running = false;
                ImGui::EndTabItem();
                
            }
            if (ImGui::BeginTabItem("Controls")) { 
                if (controlMapping.m_capture.active) {
                    controlMapping.updateCapture(0.1f);
                }
                controlMapping.renderUI();
                ImGui::EndTabItem(); 
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}