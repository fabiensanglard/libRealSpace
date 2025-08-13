#include "DebugObjectViewer.h"
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>

DebugObjectViewer::DebugObjectViewer() {

}
DebugObjectViewer::~DebugObjectViewer() {
    
}
void DebugObjectViewer::renderMenu() {
    static bool load_object = false;
    static bool view_textures = false;

    if (ImGui::BeginMenu("Object Viewer")) {
        ImGui::MenuItem("Load Object", nullptr, &load_object);
        ImGui::MenuItem("view Textures", nullptr, &view_textures);
        ImGui::EndMenu();
    }
    ImGui::Text("Object Name: %s", objs.showCases[currentObject].filename.c_str());
    if (load_object) {
        ImGui::Begin("List Of Objects");
        static ImGuiComboFlags shtsflags = 0;
        if (ImGui::BeginCombo("List des objets", nullptr, shtsflags)) {
            for (auto tre :Assets.tres) {
                for (auto entry : tre->entries) {
                    if (std::string(entry->name).find("OBJECT") != std::string::npos) {
                        // Your logic here if entry->name contains "OBJECT"
                        if (ImGui::Selectable(entry->name, false)) {
                            printf("load object");
                            RSEntity *obj = new RSEntity(&Assets);
                            obj->InitFromRAM(entry->data, entry->size);
                            objs.showCases[0].entity = obj;
                            objs.showCases[0].filename = std::string(entry->name);
                            currentObject = 0; 
                        }
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::End();
    }
    if (view_textures) {
        ImGui::Begin("List Of Textures");
        static ImGuiComboFlags shtsflags = 0;
        static RSImage *currentTexture = nullptr;
        if (ImGui::BeginCombo("List des textures", nullptr, shtsflags)) {
            for (auto tex: objs.showCases[currentObject].entity->images) {
                if (ImGui::Selectable(tex->name, false)) {
                    currentTexture = tex;
                }
            }
            ImGui::EndCombo();
        }
        if (currentTexture != nullptr) {
            ImGui::Image((void*)(intptr_t)currentTexture->GetTexture()->GetTextureID(), ImVec2(256, 256), {0, 1}, {1, 0});
        }           
        ImGui::End();
    }
}
void DebugObjectViewer::renderUI() {
    if (ImGui::BeginTabBar("Object Viewer")) {
        if (ImGui::BeginTabItem("Objects")) {
            ImGui::BeginChild("List", ImVec2(0, 400), true);
            for (auto tre :Assets.tres) {
                for (auto entry : tre->entries) {
                    if (std::string(entry->name).find("OBJECT") != std::string::npos) {
                        if (ImGui::Selectable(entry->name, false)) {
                            printf("load object");
                            RSEntity *obj = new RSEntity(&Assets);
                            obj->InitFromRAM(entry->data, entry->size);
                            objs.showCases[0].entity = obj;
                            objs.showCases[0].filename = std::string(entry->name);
                            currentObject = 0; 
                        }
                    }
                }
            }
            ImGui::EndChild();
            ImGui::BeginChild("Camera", ImVec2(0, 400), true);
            ImGui::SliderFloat("Zoom", &objs.showCases[0].cameraDist, 1.0f, 100000.0f);
            ImGui::SliderFloat("Angle Up/Down", &this->rotateUpDownAngle, -180.0f, 180.0f);
            ImGui::SliderFloat("Angle Left/Right", &this->rotateLeftRightAngle, -180.0f, 180.0f);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Details")) {
            ImGui::BeginChild("Details", ImVec2(0, 200), true);
            if (objs.showCases[currentObject].entity != nullptr) {
                ImGui::Text("Entity Name: %s", objs.showCases[currentObject].filename.c_str());
                ImGui::Text("Number of Images: %zu", objs.showCases[currentObject].entity->NumImages());
                ImGui::Text("Number of Vertices: %zu", objs.showCases[currentObject].entity->NumVertice());
                ImGui::Text("Number of UVs: %zu", objs.showCases[currentObject].entity->NumUVs());
                ImGui::Text("Number of LODs: %zu", objs.showCases[currentObject].entity->NumLods());
                ImGui::Text("Number of Triangles: %zu", objs.showCases[currentObject].entity->NumTriangles());
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "No entity loaded");
            }
            ImGui::EndChild();
            ImGui::BeginChild("Textures", ImVec2(0, 0), true);
            ImGui::Text("Textures loaded: %d", objs.showCases[currentObject].entity->images.size());
            if (ImGui::BeginTable("TexturesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Texture Name");
                ImGui::TableSetupColumn("Texture ID");
                ImGui::TableHeadersRow();
                for (const auto &texture : objs.showCases[currentObject].entity->images) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", texture->name);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Image((ImTextureID)(intptr_t)texture->GetTexture()->id, ImVec2(100, 100));
                }
                for (const auto &texture : objs.showCases[currentObject].entity->animations) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", texture->name);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Image((ImTextureID)(intptr_t)texture->id, ImVec2(100, 100));
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Properties")) {
            
            RSEntity *entity = objs.showCases[currentObject].entity;
            ImGui::BeginChild("general", ImVec2(0, 50), true);
            ImGui::Text("Entity Type: %d", entity->entity_type);
            ImGui::Text("Target Type: %d", entity->target_type);
            ImGui::EndChild();
            if (entity->wdat != nullptr) {
                ImGui::BeginChild("Weapon Data:", ImVec2(0, 200), true);
                ImGui::Text("Damage: %d", entity->wdat->damage);
                ImGui::Text("Radius: %d", entity->wdat->radius);
                ImGui::Text("Weapon ID: %d", entity->wdat->weapon_id);
                ImGui::Text("Weapon Category: %d", entity->wdat->weapon_category);
                ImGui::Text("Radar Type: %d", entity->wdat->radar_type);
                ImGui::Text("Target Range: %d", entity->wdat->target_range);
                ImGui::EndChild();
            }
            if (entity->dynn_miss != nullptr) {
                ImGui::BeginChild("Dynamic Missile Data:", ImVec2(0, 50), true);
                ImGui::Text("Turn Degree per Second: %d", entity->dynn_miss->turn_degre_per_sec);
                ImGui::Text("Velocity m/s: %d", entity->dynn_miss->velovity_m_per_sec);
                ImGui::Text("Proximity cm: %d", entity->dynn_miss->proximity_cm);
                ImGui::EndChild();
            }
            if (entity->explos != nullptr) {
                ImGui::BeginChild("Explosion Data:", ImVec2(0, 50), true);
                ImGui::Text("Explosion Name: %s", entity->explos->name.c_str());
                ImGui::Text("Explosion Position: (%d, %d)", entity->explos->x, entity->explos->y);
                if (entity->explos->objct != nullptr) {
                    ImGui::Text("Explosion Animations Frames %d", entity->explos->objct->animations.size());
                } else {
                    ImGui::Text("Explosion Object: None");
                }
                ImGui::EndChild();
            }
            if (entity->jdyn != nullptr) {
                ImGui::BeginChild("JDYN Data:", ImVec2(0, 0), true);
                ImGui::Text("Fuel: %d", entity->jdyn->FUEL);
                ImGui::Text("Roll Rate: %d", entity->jdyn->ROLL_RATE);
                ImGui::Text("Max Roll Rate: %d", entity->jdyn->ROLL_RATE_MAX);
                ImGui::Text("Lift Speed: %d", entity->jdyn->LIFT_SPD);
                ImGui::Text("Drag: %d", entity->jdyn->DRAG);
                ImGui::Text("Lift: %d", entity->jdyn->LIFT);
                ImGui::Text("Aileron: %d", entity->jdyn->aileron);
                ImGui::Text("Gouverne: %d", entity->jdyn->gouverne);
                ImGui::Text("Max G: %d", entity->jdyn->MAX_G);
                ImGui::Text("Twist Rate: %d", entity->jdyn->TWIST_RATE);
                ImGui::Text("Max Twist Rate: %d", entity->jdyn->TWIST_RATE_MAX);
                ImGui::Text("Wing Area: %.2f", entity->wing_area);
                ImGui::Text("Gravity: %s", entity->gravity ? "Enabled" : "Disabled");
                ImGui::Text("Weight in kg: %d", entity->weight_in_kg);
                ImGui::Text("Thrust in Newton: %d", entity->thrust_in_newton);
                ImGui::Text("Drag: %d", entity->drag);
                ImGui::EndChild();
            }
            
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}