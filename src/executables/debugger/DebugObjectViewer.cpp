#include "DebugObjectViewer.h"
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>

DebugObjectViewer::DebugObjectViewer() {
    vertices.clear();
    vertices.shrink_to_fit();
}
DebugObjectViewer::~DebugObjectViewer() {
    
}
void DebugObjectViewer::runFrame()  {
    /**/
    // Ok now time to draw the model
    glClear(GL_DEPTH_BUFFER_BIT);

    uint32_t currentTime = SDL_GetTicks();
    uint32_t totalTime = currentTime - startTime;

    RSShowCase showCase = objs.showCases[currentObject];

    Point3D newPosition;
    if (autoRotate) {
        newPosition.x = showCase.cameraDist / 150 * cosf(totalTime / 2000.0f);
        newPosition.y = showCase.cameraDist / 350;
        newPosition.z = showCase.cameraDist / 150 * sinf(totalTime / 2000.0f);
    } else {
        newPosition.x = showCase.cameraDist / 150;
        newPosition.y = showCase.cameraDist / 350;
        newPosition.z = showCase.cameraDist / 150;
    }

    Renderer.getCamera()->SetPosition(&newPosition);
    Point3D lookAt = {0, 0, 0};
    Renderer.getCamera()->lookAt(&lookAt);
    if (moovingLight) {
        light.x = 4 * cosf(-1 * totalTime / 20000.0f);
        light.y = 4;
        light.z = 4 * sinf(-1 * totalTime / 20000.0f);
    }
    Renderer.setLight(&light);

    glMatrixMode(GL_PROJECTION);
    Matrix *projection = Renderer.getCamera()->getProjectionMatrix();
    glLoadMatrixf(projection->ToGL());

    glMatrixMode(GL_MODELVIEW);
    Matrix *view = Renderer.getCamera()->getViewMatrix();
    glLoadMatrixf(view->ToGL());
    glPushMatrix();
    glRotatef(this->rotateLeftRightAngle, 1, 0, 0);
    glPushMatrix();
    glRotatef(this->rotateUpDownAngle, 0, 0, 1);

    glScalef(1 / this->zoomFactor, 1 / this->zoomFactor, 1 / this->zoomFactor);
    if (objs.showCases[currentObject].entity->triangles.size() > 0) {
        Renderer.drawModel(objs.showCases[currentObject].entity, lodLevel);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);
        glColor3f(0.0f, 1.0f, 0.0f); // Vert pour les lignes
        
        
        const Lod* lod = &objs.showCases[currentObject].entity->lods[lodLevel];
        RSEntity *entity = objs.showCases[currentObject].entity;
        
        
        for (int ti = 0; ti < lod->numTriangles; ti++) {
            int triangle_id = lod->triangleIDs[ti];
            if (entity->attrs.find(lod->triangleIDs[ti]) != entity->attrs.end() &&
                entity->attrs[lod->triangleIDs[ti]]->type != 'T') {
                continue;
            } else if (entity->attrs.find(lod->triangleIDs[ti]) != entity->attrs.end()) {
                triangle_id = entity->attrs[lod->triangleIDs[ti]]->id;
            }
            const Triangle& triangle = entity->triangles[triangle_id];
            glBegin(GL_TRIANGLES);
            for (int i = 0; i < 3; i++) {
                Vector3D vert = entity->vertices[triangle.ids[i]];
                glVertex3f(vert.x, vert.y, vert.z);
            }
            glEnd();
        }
        for (int ti = 0; ti < lod->numTriangles; ti++) {
            int quad_id = 0;
            int prop1 = 0;
            int prop2 = 0;
            if (entity->attrs.find(lod->triangleIDs[ti]) != entity->attrs.end() &&
                entity->attrs[lod->triangleIDs[ti]]->type != 'Q') {
                continue;
            } else if (entity->attrs.find(lod->triangleIDs[ti]) != entity->attrs.end()) {
                quad_id = entity->attrs[lod->triangleIDs[ti]]->id;
                prop1 = entity->attrs[lod->triangleIDs[ti]]->props1;
                prop2 = entity->attrs[lod->triangleIDs[ti]]->props2;
            } else {
                continue;
            }
            
            const Quads *triangle = entity->quads[quad_id];
            glColor3f(1.0f, 0.0f, 1.0f);
            bool twoSided = false;
            twoSided = prop2 == 1;
            if (twoSided) {
                glColor3f(1.0f, 1.0f, 0.0f);
                glDisable(GL_CULL_FACE);
            }
            glBegin(GL_QUADS);
            for (int i = 0; i < 4; i++) {
                Vector3D vert = entity->vertices[triangle->ids[i]];
                glVertex3f(vert.x, vert.y, vert.z);
            }
            glEnd();
            if (twoSided) {
                glEnable(GL_CULL_FACE);
            }
        }
        
        
        // Dessiner les vertices
        glPointSize(5.0f);
        glColor3f(1.0f, 0.0f, 0.0f); // Rouge pour les vertices
        glBegin(GL_POINTS);
        for (int ti = 0; ti < lod->numTriangles; ti++) {
            int triangle_id = lod->triangleIDs[ti];
            if (entity->attrs.find(lod->triangleIDs[ti]) != entity->attrs.end() &&
                entity->attrs[lod->triangleIDs[ti]]->type != 'T') {
                continue;
            } else if (entity->attrs.find(lod->triangleIDs[ti]) != entity->attrs.end()) {
                triangle_id = entity->attrs[lod->triangleIDs[ti]]->id;
            }
            const Triangle& triangle = entity->triangles[triangle_id];
            for (int i = 0; i < 3; i++) {
                Vector3D vert = entity->vertices[triangle.ids[i]];
                glVertex3f(vert.x, vert.y, vert.z);
            }
        }
        glEnd();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glLineWidth(1.0f);
        
    } else if (objs.showCases[currentObject].entity->animations.size() > 0) {
        this->fps++;
        if (this->fps > 12) {
            this->fps = 0;
            this->current_frame = (this->current_frame + 1) % objs.showCases[currentObject].entity->animations.size();
        }
        Renderer.drawBillboard(
            {0, 0, 0},
            objs.showCases[currentObject].entity->animations[this->current_frame],
            1.0f / this->zoomFactor
        );
    } else {
        printf("No model to draw for %s\n", objs.showCases[currentObject].displayName.c_str());
    }
    
    glPopMatrix();
    glPopMatrix();
    glDisable(GL_DEPTH_TEST);
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
                            RSEntity *obj = new RSEntity();
                            obj->InitFromRAM(entry->data, entry->size, entry->name);
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
            ImGui::Image((void*)(intptr_t)currentTexture->GetTexture()->getTextureID(), ImVec2(256, 256), {0, 1}, {1, 0});
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
                            RSEntity *obj = new RSEntity();
                            obj->InitFromRAM(entry->data, entry->size, entry->name);
                            objs.showCases[0].entity = obj;
                            objs.showCases[0].filename = std::string(entry->name);
                            currentObject = 0; 
                        }
                    }
                }
            }
            ImGui::EndChild();
            ImGui::BeginChild("Camera", ImVec2(0, 150), true);
            ImGui::Checkbox("Auto Rotate", &autoRotate);
            ImGui::SliderFloat("Zoom", &objs.showCases[0].cameraDist, 1.0f, 100000.0f);
            ImGui::SliderFloat("Angle Up/Down", &this->rotateUpDownAngle, -180.0f, 180.0f);
            ImGui::SliderFloat("Angle Left/Right", &this->rotateLeftRightAngle, -180.0f, 180.0f);
            ImGui::EndChild();
            ImGui::BeginChild("Light", ImVec2(0, 150), true);
            ImGui::Checkbox("Mooving Light", &moovingLight);
            ImGui::Text("Light Position: (%.2f, %.2f, %.2f)", light.x, light.y, light.z);
            ImGui::SliderFloat("Light X", &light.x, -10.0f, 10.0f);
            ImGui::SliderFloat("Light Y", &light.y, -10.0f, 10.0f);
            ImGui::SliderFloat("Light Z", &light.z, -10.0f, 10.0f);
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
            ImGui::BeginChild("Textures", ImVec2(0, 400), true);
            ImGui::Text("Textures loaded: %zu", objs.showCases[currentObject].entity->images.size());
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
            ImGui::BeginChild("Triangles", ImVec2(0, 400), true);
            int tri_count = 0;
            const Lod* lod = &objs.showCases[currentObject].entity->lods[lodLevel];
            RSEntity *entity = objs.showCases[currentObject].entity;
            for (int ti = 0; ti < lod->numTriangles; ti++) {
                if (objs.showCases[currentObject].entity->attrs[lod->triangleIDs[ti]]->type != 'T') {
                    continue;
                }
                const Triangle& triangle = objs.showCases[currentObject].entity->triangles[lod->triangleIDs[ti]];
                for (const auto &vertex : triangle.ids) {
                    ImGui::Text("  Vertex ID: %d", vertex);
                    Vector3D vert=entity->vertices[vertex];
                    ImGui::Text("    Position: (%.2f, %.2f, %.2f)", vert.x, vert.y, vert.z);
                    this->vertices.push_back(vert);
                }
                if (ImGui::TreeNode(("Triangle " + std::to_string(lod->triangleIDs[ti])).c_str())) {
                    ImGui::Text("Triangle ID: %d", lod->triangleIDs[ti]);
                    ImGui::Text("Property: %d", triangle.property);
                    for (int i=0; i<3; i++) {
                        ImGui::Text("Flag %d: %d", i, triangle.flags[i]);
                    }
                    ImGui::Text("Color ID: %d", triangle.color);
                    Texel *color = palette.GetRGBColor(triangle.color);
                    ImGui::Text("Color: R: %d, G: %d, B: %d, A: %d", color->r, color->g, color->b, color->a);
                    float colorValue[3] = {color->r / 255.0f, color->g / 255.0f, color->b / 255.0f};
                    ImGui::ColorPicker3("Color Picker", colorValue, ImGuiColorEditFlags_NoInputs);
                    
                    if (entity->attrs.find(lod->triangleIDs[ti]) == entity->attrs.end()) {
                        ImGui::Text("No attribute for this triangle");
                    } else {
                        Attr *attr = entity->attrs[lod->triangleIDs[ti]];
                        if (attr != nullptr) {
                            ImGui::Text("Attribute props1: %d", attr->props1);
                            ImGui::Text("Attribute props2: %d", attr->props2);
                            
                            ImGui::Text("Attribute ID: %d", attr->id);
                            ImGui::Text("Attribute Type: %d", attr->type);
                        }
                    }
                    ImGui::Text("Vertices: ");
                    for (const auto &vertex : triangle.ids) {
                        ImGui::Text("  Vertex ID: %d", vertex);
                        Vector3D vert=entity->vertices[vertex];
                        ImGui::Text("    Position: (%.2f, %.2f, %.2f)", vert.x, vert.y, vert.z);
                        this->vertices.push_back(vert);
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::EndChild();
            
            ImGui::BeginChild("Quads", ImVec2(0, 400), true);
            int quad_count = 0;
            for (auto quad : objs.showCases[currentObject].entity->quads) {
                if (ImGui::TreeNode(("Quad " + std::to_string(quad_count++)).c_str())) {
                    ImGui::Text("Quad ID: %d", quad_count);
                    ImGui::Text("Property: %d", quad->property);
                    for (int i=0; i<4; i++) {
                        ImGui::Text("Flag %d: %d", i, quad->flags[i]);
                    }
                    ImGui::Text("Color ID: %d", quad->color);
                    Texel *color = palette.GetRGBColor(quad->color);
                    ImGui::Text("Color: R: %d, G: %d, B: %d, A: %d", color->r, color->g, color->b, color->a);
                    RSEntity *entity = objs.showCases[currentObject].entity;
                    if (entity->attrs.find(quad_count - 1) == entity->attrs.end()) {
                        ImGui::Text("No attribute for this quad");
                    } else {
                        Attr *attr = entity->attrs[quad_count - 1];
                        if (attr != nullptr) {
                            ImGui::Text("Attribute props1: %d", attr->props1);
                            ImGui::Text("Attribute props2: %d", attr->props2);
                            ImGui::Text("Attribute ID: %d", attr->id);
                            ImGui::Text("Attribute Type: %d", attr->type);
                        }
                    }
                    ImGui::Text("Vertices: ");
                    for (const auto &vertex : quad->ids) {
                        ImGui::Text("  Vertex ID: %d", vertex);
                        Vector3D vert=entity->vertices[vertex];
                        ImGui::Text("    Position: (%.2f, %.2f, %.2f)", vert.x, vert.y, vert.z);
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Properties")) {
            
            RSEntity *entity = objs.showCases[currentObject].entity;
            ImGui::InputInt("LOD Level", &lodLevel);
            ImGui::BeginChild("general", ImVec2(0, 0), true);
            ImGui::Text("Entity Type: %d", entity->entity_type);
            ImGui::Text("Target Type: %d", entity->target_type);
            if (entity->wdat != nullptr) {
                if (ImGui::TreeNode("Weapon Data")) {
                    ImGui::Text("Damage: %d", entity->wdat->damage);
                    ImGui::Text("Radius: %d", entity->wdat->radius);
                    ImGui::Text("Weapon ID: %d", entity->wdat->weapon_id);
                    ImGui::Text("Weapon Category: %d", entity->wdat->weapon_category);
                    ImGui::Text("Target Domain: %d", entity->wdat->target_domain);
                    ImGui::Text("Weapon Aspect: %d", entity->wdat->weapon_aspec);
                    ImGui::Text("Target Range: %d", entity->wdat->target_range);
                    ImGui::Text("Tracking Cone: %d", entity->wdat->tracking_cone);
                    ImGui::Text("Effective Range: %d", entity->wdat->effective_range);
                    ImGui::TreePop();
                }
            }
            if (entity->dynn_miss != nullptr) {
                if (ImGui::TreeNode("Dynamic Missile Data")) {
                    ImGui::Text("Turn Degree per Second: %.2f", entity->dynn_miss->turn_degre_per_sec);
                    ImGui::Text("Velocity m/s: %.2f", entity->dynn_miss->velovity_m_per_sec);
                    ImGui::Text("Proximity cm: %d", entity->dynn_miss->proximity_cm);
                    ImGui::TreePop();
                }
            }
            if (entity->explos != nullptr) {
                if (ImGui::TreeNode("Explosion Data")) {
                    ImGui::Text("Explosion Name: %s", entity->explos->name.c_str());
                    ImGui::Text("Explosion Position: (%d, %d)", entity->explos->x, entity->explos->y);
                    if (entity->explos->objct != nullptr) {
                        ImGui::Text("Explosion Animations Frames %zu", entity->explos->objct->animations.size());
                    } else {
                        ImGui::Text("Explosion Object: None");
                    }
                    ImGui::TreePop();
                }
            }
            if (entity->destroyed_object != nullptr) {
                ImGui::Text("Destroyed Object: %s", entity->destroyed_object_name.c_str());
            } else {
                ImGui::Text("Destroyed Object: None");
            }
            if (entity->jdyn != nullptr) {
                if (ImGui::TreeNode("Jet Dynamics")) {
                    ImGui::Text("Fuel: %d", entity->jdyn->fuel_capacity);
                    ImGui::Text("Aileron: %d", entity->jdyn->pitch_stick_gain);
                    ImGui::Text("Gouverne: %d", entity->jdyn->yaw_authority);
                    ImGui::Text("Max G: %d", entity->jdyn->max_g);
                    ImGui::Text("Wing Area: %.2f", entity->wing_area);
                    ImGui::Text("Gravity: %s", entity->gravity ? "Enabled" : "Disabled");
                    ImGui::Text("Weight in kg: %d", entity->weight_in_kg);
                    ImGui::Text("Thrust in Newton: %d", entity->thrust_in_newton);
                    ImGui::Text("Drag: %d", entity->drag);
                    ImGui::TreePop();
                }
            }
            if (entity->chld.size() > 0) {
                if (ImGui::TreeNode("Child Objects")) {
                    for (const auto &chld : entity->chld) {
                        ImGui::Text("Child Name: %s", chld->name.c_str());
                        ImGui::Text("Position: (%d, %d, %d)", chld->x, chld->y, chld->z);
                        if (chld->objct != nullptr) {
                            ImGui::Text("Child Object Frames: %zu", chld->objct->animations.size());
                        } else {
                            ImGui::Text("Child Object: None");
                        }
                    }
                    ImGui::TreePop();
                }
            }
            if (entity->weaps.size() > 0) {
                if (ImGui::TreeNode("Weapons")) {
                    for (const auto &weap : entity->weaps) {
                        ImGui::Text("Weapon Name: %s", weap->name.c_str());
                        ImGui::Text("Number of Weapons: %d", weap->nb_weap);
                        if (weap->objct != nullptr) {
                            ImGui::Text("Weapon Object Frames: %zu", weap->objct->animations.size());
                        } else {
                            ImGui::Text("Weapon Object: None");
                        }
                    }
                    ImGui::TreePop();
                }
            }
            if (entity->sysm.size() > 0) {
                if (ImGui::TreeNode("Damages System Data")) {
                    for (const auto &sys : entity->sysm) {
                        ImGui::Text("System Name: %s", sys.first.c_str());
                        for (const auto &attr : sys.second) {
                            ImGui::Text("  Attribute: %s, Value: %d", attr.first.c_str(), attr.second);
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::Text("Health: %d", entity->health);
            if (entity->swpn_data != nullptr) {
                if (ImGui::TreeNode("Surface Weapon Data")) {
                    ImGui::Text("Weapon Name: %s", entity->swpn_data->weapon_name.c_str());
                    ImGui::Text("Weapons Round: %d", entity->swpn_data->weapons_round);
                    ImGui::Text("Detection Range: %d", entity->swpn_data->detection_range);
                    ImGui::Text("Effective Range: %d", entity->swpn_data->effective_range);
                    ImGui::Text("Unknown1: %d", entity->swpn_data->unknown1);
                    ImGui::Text("Unknown2: %d", entity->swpn_data->unknown2);
                    ImGui::Text("Unknown3: %d", entity->swpn_data->unknown3);
                    ImGui::Text("Max Simultaneous Shots: %d", entity->swpn_data->max_simultaneous_shots);
                    ImGui::Text("Weapons Round2: %d", entity->swpn_data->weapons_round2);
                    ImGui::Text("Unknown4: %d", entity->swpn_data->unknown4);
                    ImGui::TreePop();
                }
            }
            if (entity->radar_signature != nullptr) {
                if (ImGui::TreeNode("Radar Signature")) {
                    ImGui::Text("Unknown1: %d", entity->radar_signature->unknown1);
                    ImGui::Text("Unknown2: %d", entity->radar_signature->unknown2);
                    ImGui::Text("Unknown3: %d", entity->radar_signature->unknown3);
                    ImGui::TreePop();
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}