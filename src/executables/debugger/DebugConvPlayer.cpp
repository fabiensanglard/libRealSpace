#include "DebugConvPlayer.h"
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>

void DebugConvPlayer::CheckFrameExpired(void) {
    if (this->paused == true) {
        return;
    }
    SCConvPlayer::CheckFrameExpired();
}

DebugConvPlayer::DebugConvPlayer() {
    this->paused = true;
}

DebugConvPlayer::~DebugConvPlayer() {
}

void DebugConvPlayer::renderMenu() {
}

void DebugConvPlayer::renderUI() {
    static std::vector<GLuint> s_PrevFrameGLTex;
    static std::vector<GLuint> s_CurrentFrameGLTex;
    // Détruire les textures de la frame précédente (elles ont été rendues)
    if (!s_PrevFrameGLTex.empty()) {
        for (GLuint id : s_PrevFrameGLTex) {
            glDeleteTextures(1, &id);
        }
        s_PrevFrameGLTex.clear();
    }
    // Préparer la liste pour cette frame
    s_PrevFrameGLTex.swap(s_CurrentFrameGLTex); // s_CurrentFrameGLTex devient vide

    if (ImGui::BeginTabBar("Conversation")) {
        if (ImGui::BeginTabItem("Conversation Data")) {
            ImGui::Button(this->paused ? "Play" : "Pause");
            ImGui::SameLine();
            if (ImGui::Button("Next Frame")) {
                this->paused = true;
                this->current_frame->SetExpired(true);
            }
            ImGui::Text("Conversation ID: %d", this->conversationID);
            ImGui::Text("Number of frames: %d", (int)this->conversation_frames.size());
            if (this->current_frame != nullptr) {
                ImGui::Text("Current Frame Type: %d", this->current_frame->mode);
                ImGui::Text("Current Frame Time: %d", this->current_frame->creationTime);
                ImGui::Text("Current Frame Text: %s", this->current_frame->text);
            }
            int frameNumber = 0;
            for (auto frame : this->conversation_frames) {
                frameNumber++;
                if (ImGui::TreeNodeEx((void *)(intptr_t)frameNumber, ImGuiTreeNodeFlags_DefaultOpen, "Frame ID %d", frameNumber)) {
                    ImGui::Text("Frame Type: %d", frame->mode);
                    ImGui::Text("Frame Time: %d", frame->creationTime);
                    ImGui::Text("Frame Text: %s", frame->text);
                    if (frame->face != nullptr) {
                        ImGui::Text("Face Position: %d", frame->facePosition);
                        ImGui::Text("Face Expression: %d", frame->face_expression);
                        ImGui::Text("Face Palette ID: %d", frame->facePaletteID);
                        FrameBuffer *fb = new FrameBuffer(320, 200);
                        fb->FillWithColor(223);
                        fb->DrawShape(frame->face->appearances->GetShape(1));
                        Texel* tex = fb->getTexture(&this->palette);

                        // Création texture OpenGL
                        GLuint glTex = 0;
                        glGenTextures(1, &glTex);
                        glBindTexture(GL_TEXTURE_2D, glTex);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);

                        // Affichage dans ImGui
                        ImGui::Image((ImTextureID)(intptr_t)glTex, ImVec2(320, 200));
                        delete fb;
                        delete tex;
                        s_CurrentFrameGLTex.push_back(glTex);

                    }
                    if (frame->participants.size() > 0) {
                        if (ImGui::TreeNodeEx("Participants", ImGuiTreeNodeFlags_DefaultOpen)) {
                            for (auto part : frame->participants) {
                                ImGui::Text("Participant: %s", part->name);
                                FrameBuffer *fb = new FrameBuffer(320, 200);
                                fb->FillWithColor(223);
                                fb->DrawShape(part->appearances->GetShape(0));
                                Texel* tex = fb->getTexture(&this->palette);
                                // Création texture OpenGL
                                GLuint glTex = 0;
                                glGenTextures(1, &glTex);
                                glBindTexture(GL_TEXTURE_2D, glTex);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
                                // Affichage dans ImGui
                                ImGui::Image((ImTextureID)(intptr_t)glTex, ImVec2(320, 200));
                                delete fb;
                                delete tex;
                                s_CurrentFrameGLTex.push_back(glTex);

                            }
                            ImGui::TreePop();
                        }
                    }
                    if (frame->bgLayers != nullptr && frame->bgLayers->size() > 0) {
                        if (ImGui::TreeNodeEx("Background Layers", ImGuiTreeNodeFlags_DefaultOpen)) {
                            for (auto layer : *frame->bgLayers) {
                                FrameBuffer *fb = new FrameBuffer(320, 200);
                                fb->FillWithColor(223);
                                fb->DrawShape(layer);
                                Texel* tex = fb->getTexture(&this->palette);

                                // Création texture OpenGL
                                GLuint glTex = 0;
                                glGenTextures(1, &glTex);
                                glBindTexture(GL_TEXTURE_2D, glTex);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);

                                // Affichage dans ImGui
                                ImGui::Image((ImTextureID)(intptr_t)glTex, ImVec2(320, 200));
                                delete fb;
                                delete tex;
                                s_CurrentFrameGLTex.push_back(glTex);

                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
