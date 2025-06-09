#include "DebugStrike.h"
#include <cctype>
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>
#include <tuple>
#include <optional>
#include <cmath>


DebugStrike::DebugStrike() : SCStrike(){
   
}

DebugStrike::~DebugStrike() {}

void DebugStrike::setMission(std::string mission_name) {
    SCStrike::setMission(mission_name.c_str());
}

void DebugStrike::init() {
    SCStrike::init();
}
void DebugStrike::showOffCamera() {
    ImVec2 avail_size = ImGui::GetContentRegionAvail();
    ImGui::Image((void*)(intptr_t)Renderer.texture, avail_size, {0, 1}, {1, 0});
}
void DebugStrike::simInfo() {
    ImGui::Text("Speed %d, Altitude %.0f, Heading %.0f", this->player_plane->airspeed, this->new_position.y,
        this->player_plane->azimuthf);
    ImGui::Text("Throttle %d", this->player_plane->GetThrottle());
    ImGui::Text("Control Stick %d %d", this->player_plane->control_stick_x, this->player_plane->control_stick_y);
    

    ImGui::Text("Pitch %.3f, Yaw %.3f, roll %.3f", this->player_plane->elevationf, this->player_plane->azimuthf,
            this->player_plane->twist);
    ImGui::Text("Pich input %.3f, Roll input %d, Yaw input %.3f", this->player_plane->elevation_speedf, this->player_plane->roll_speed, this->player_plane->azimuth_speedf);
    ImGui::Text("Y %.3f, On ground %d", this->player_plane->y, this->player_plane->on_ground);
    ImGui::Text("flight [roller:%4f, elevator:%4f, rudder:%4f]", this->player_plane->rollers,
            this->player_plane->elevator, this->player_plane->rudder);
    ImGui::Text("Acceleration (vx,vy,vz) [%.3f ,%.3f ,%.3f ]", this->player_plane->vx, this->player_plane->vy,
            this->player_plane->vz);
    ImGui::Text("Acceleration (ax,ay,az) [%.3f ,%.3f ,%.3f ]", this->player_plane->ax, this->player_plane->ay,
            this->player_plane->az);
    ImGui::Text("Lift %.3f", this->player_plane->lift);
    ImGui::Text("Gravity %.3f", this->player_plane->gravity);
    ImGui::Text("Drag %.3f", this->player_plane->drag);
    ImGui::Text("Inv mass %.8f", this->player_plane->inverse_mass);
    ImGui::Text("Thrust %.3f", this->player_plane->thrust_force);
    ImGui::Text("ptw");
    for (int o = 0; o < 4; o++) {
    ImGui::Text("PTW[%d]=[%f,%f,%f,%f]", o, this->player_plane->ptw.v[o][0], this->player_plane->ptw.v[o][1],
                this->player_plane->ptw.v[o][2], this->player_plane->ptw.v[o][3]);
    }
    ImGui::Text("incremental");
    for (int o = 0; o < 4; o++) {
    ImGui::Text("INC[%d]=[%f,%f,%f,%f]", o, this->player_plane->incremental.v[o][0],
                this->player_plane->incremental.v[o][1], this->player_plane->incremental.v[o][2],
                this->player_plane->incremental.v[o][3]);
    }
}
void DebugStrike::loadPlane() {
    std::vector<std::string> planes;
    planes = {
        "747",
        "A-10",
        "AWACS",
        "B-1B",
        "C130DES",
        "C130GRN",
        "F-4",
        "F-15",
        "F-16DES",
        "F-16GRAY",
        "F-18",
        "F-22",
        "F-117",
        "LEARJET",
        "MIG21",
        "MIG29",
        "MIRAGE",
        "SU27",
        "TORNCG",
        "TU-20",
        "YF23",
        "F4F3",
        "TBD",
        "SBD",
        "TBF",
        "HELLCAT",
        "HELLDIVE",
        "CORSAIR",
        "BEARCAT",
        "ZERO",
        "KATETORP",
        "VAL",
        "CLAUDE",
        "FRANK",
        "JUDY",
        "BETTY",
        "EMILY",
        "BAKA",
        "F4F4",
    };
    static float envergure = 0.0f;
    static float thrust = 0.0f;
    static float weight = 0.0f;
    static float fuel = 0.0f;
    static float surface = 0.0f;
    static float wing_aspec_ratio = 0.0f;
    static float ie_pi_AR = 0.0f;
    static float roll_rate_max = 0.0f;
    static float pitch_rate_max = 0.0f;
    static std::string plane_name = "";
    static bool simple_simulation = false;
    
    if (ImGui::BeginCombo("Plane", plane_name.c_str(), 0)) {
        for (auto plane : planes) {
            if (ImGui::Selectable(plane.c_str(), !plane_name.compare(plane))) {
                plane_name = plane;
            }
            if (!plane_name.compare(plane)) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    ImGui::Checkbox("Simple Sim", &simple_simulation);
    if (ImGui::Button("Load") && plane_name != "") {
        //this->plane_to_load = planes.indexOf(plane);
        RSEntity *plane_to_load = new RSEntity(&Assets);
        TreEntry *entry = Assets.GetEntryByName(Assets.object_root_path + plane_name + ".IFF");
        plane_to_load->InitFromRAM(entry->data, entry->size);
        BoudingBox *bb = plane_to_load->GetBoudingBpx();
        envergure = (bb->max.z - bb->min.z) / 2.0f;
        surface = plane_to_load->wing_area;
        
        thrust = plane_to_load->thrust_in_newton * 0.153333333f;
        weight = plane_to_load->weight_in_kg * 2.208588957f;
        fuel = plane_to_load->jdyn->FUEL * 2.208588957f;
        
        wing_aspec_ratio = (envergure * envergure) / surface ;
        ie_pi_AR = 4000.0f/plane_to_load->drag;
        roll_rate_max = plane_to_load->jdyn->ROLL_RATE;
        pitch_rate_max = plane_to_load->jdyn->TWIST_RATE;
        SCPlane *new_plane = nullptr;
        if (simple_simulation) {
            new_plane = new SCJdynPlane(
                10.0f,
                -7.0f,
                40.0f,
                40.0f,
                plane_to_load->jdyn->TWIST_RATE,
                plane_to_load->jdyn->ROLL_RATE,
                surface,
                weight,
                fuel,
                thrust,
                envergure,
                0.83f,
                120,
                this->current_mission->area,
                player_plane->x,
                player_plane->y,
                player_plane->z
            );
        } else {
            surface = surface * 10.7639f;
            envergure = envergure * 3.28084f;
            new_plane = new SCPlane(
                10.0f,
                -7.0f,
                40.0f,
                40.0f,
                plane_to_load->jdyn->TWIST_RATE,
                plane_to_load->jdyn->ROLL_RATE,
                surface,
                weight,
                fuel,
                thrust,
                envergure,
                0.83f,
                120,
                this->current_mission->area,
                player_plane->x,
                player_plane->y,
                player_plane->z
            );
        }
        
        new_plane->simple_simulation = false;
        new_plane->object = player_plane->object;
        new_plane->object->entity = plane_to_load;
        new_plane->azimuthf = player_plane->azimuthf;
        new_plane->yaw = player_plane->yaw;
        this->player_plane = new_plane;
        this->current_mission->player->plane = this->player_plane;

        this->cockpit->player_plane = this->player_plane;
    }
    ImGui::Text("Plane name: %s", plane_name.c_str());
    ImGui::Text("Envergure: %.2f", envergure);
    ImGui::Text("Thrust: %.2f", thrust);
    ImGui::Text("Weight: %.2f", weight);
    ImGui::Text("Fuel: %.2f", fuel);
    ImGui::Text("Surface: %.2f", this->player_plane->s);
    ImGui::Text("Wing aspect ratio: %.2f", wing_aspec_ratio);
    ImGui::Text("Induced efficiency: %.2f", ie_pi_AR);
    ImGui::Text("Roll rate max: %.2f", roll_rate_max);
    ImGui::Text("Pitch rate max: %.2f", pitch_rate_max);
}
void DebugStrike::simConfig() {
    static bool azymuth_control = false;
    static int altitude = 0;
    static int azimuth = 0;
    static int throttle = 0;
    static int speed = 0;
    static bool go_to_nav = false;
    ImGui::DragInt("set altitude", &altitude, 100, 0, 30000);
    ImGui::DragInt("set throttle", &throttle, 10, 0, 100);
    ImGui::DragInt("set azimuth", &azimuth, 1, 0, 360);
    ImGui::DragInt("set speed", &speed, 2, 0, 30);
    if (ImGui::Button("set")) {
        this->player_plane->vz = (float) -speed;
        this->player_plane->y = (float) altitude;
        this->player_plane->last_py = this->player_plane->y;
        this->player_plane->rollers = 0;
        this->player_plane->roll_speed = 0;
        this->player_plane->elevation_speedf = 0;
        this->player_plane->elevator = 0;
        this->player_plane->SetThrottle(throttle);
        this->player_plane->ptw.Identity();
        this->player_plane->ptw.translateM(this->player_plane->x, this->player_plane->y, this->player_plane->z);

        this->player_plane->ptw.rotateM(tenthOfDegreeToRad(azimuth * 10.0f), 0, 1, 0);
        this->player_plane->ptw.rotateM(tenthOfDegreeToRad(this->player_plane->elevationf), 1, 0, 0);
        this->player_plane->ptw.rotateM(tenthOfDegreeToRad(this->player_plane->twist), 0, 0, 1);
        this->player_plane->Simulate();

        this->pause_simu = true;
    }
    ImGui::SameLine();
    ImGui::PushID(1);
    if (this->autopilot) {
        this->pilot.target_speed = -speed;
        this->pilot.target_climb = altitude;
        this->pilot.target_azimut = (float) azimuth;
        this->pilot.AutoPilot();
        ImGui::PushStyleColor(ImGuiCol_Button,
                                (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
    }
    if (ImGui::Button("Autopilot")) {
        this->pilot.plane = this->player_plane;
        this->autopilot = !this->autopilot;
        go_to_nav = false;
    }
    if (go_to_nav) {
        this->pilot.plane = this->player_plane;
        this->pilot.target_speed = -speed;
        this->pilot.SetTargetWaypoint(this->current_mission->waypoints[this->nav_point_id]->spot->position);
        this->pilot.AutoPilot();
    }
    ImGui::SameLine();
    if (ImGui::Button("Go To Waypoint")) {
        this->pilot.plane = this->player_plane;
        this->pilot.SetTargetWaypoint(this->current_mission->waypoints[this->nav_point_id]->spot->position);
        go_to_nav = !go_to_nav;
        this->autopilot = false;
    }
    
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    float azimut_diff = azimuth - (360 - (this->player_plane->azimuthf / 10.0f));

    if (azimut_diff > 180.0f) {
        azimut_diff -= 360.0f;
    } else if (azimut_diff < -180.0f) {
        azimut_diff += 360.0f;
    }

    const float max_twist_angle = 80.0f;
    const float Kp = 3.0f;

    float target_twist_angle = Kp * azimut_diff;
    float current_twist = 360 - this->player_plane->twist / 10.0f;

    if (current_twist > 180.0f) {
        current_twist -= 360.0f;
    } else if (current_twist < -180.0f) {
        current_twist += 360.0f;
    }

    if (target_twist_angle > 180.0f) {
        target_twist_angle -= 360.0f;
    } else if (target_twist_angle < -180.0f) {
        target_twist_angle += 360.0f;
    }

    if (target_twist_angle > max_twist_angle) {
        target_twist_angle = max_twist_angle;
    } else if (target_twist_angle < -max_twist_angle) {
        target_twist_angle = -max_twist_angle;
    }

    ImGui::Text("Current diff %f ", current_twist);
    ImGui::Text("azymuth diff %f", azimut_diff);
    ImGui::Text("target twist %f", target_twist_angle);
    ImGui::Text("Twist to go %f", current_twist - target_twist_angle);

    if (azimut_diff > 0) {
        ImGui::Text("Go right");

        if (current_twist - target_twist_angle < 0) {
            ImGui::SameLine();
            ImGui::Text("Push RIGHT");
        } else {
            ImGui::SameLine();
            ImGui::Text("Let go the stick");
        }
        if (azymuth_control) {
            if (current_twist - target_twist_angle < 0) {
                this->player_plane->control_stick_x = 50;
            } else {
                this->player_plane->control_stick_x = 0;
            }
        }
    } else {

        ImGui::Text("Go left");
        if (current_twist - target_twist_angle > 0) {
            ImGui::SameLine();
            ImGui::Text("Push LEFT");
        } else {
            ImGui::SameLine();
            ImGui::Text("Let go the stick");
        }
        if (azymuth_control) {
            if (current_twist - target_twist_angle > 0) {
                this->player_plane->control_stick_x = -50;
            } else {
                this->player_plane->control_stick_x = 0;
            }
        }
    }

    ImGui::PushID(1);
    if (azymuth_control) {
        if (azimut_diff > 0) {

        } else {
        }
        ImGui::PushStyleColor(ImGuiCol_Button,
                                (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
    }
    if (ImGui::Button("Azymuth control")) {
        azymuth_control = !azymuth_control;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    Vector3D target = {this->player_plane->x, (float) altitude, (this->player_plane->z + 60 * this->player_plane->vz)};
    Vector3D current_position = {this->player_plane->x, this->player_plane->y, this->player_plane->z};

    float target_elevation = atan2(this->player_plane->z - (this->player_plane->z + (60 * this->player_plane->vz)),
                                    this->player_plane->y - altitude);
    target_elevation = target_elevation * 180.0f / (float)M_PI;

    if (target_elevation > 180.0f) {
        target_elevation -= 360.0f;
    } else if (target_elevation < -180.0f) {
        target_elevation += 360.0f;
    }
    target_elevation = target_elevation - 90.0f;

    ImGui::Text("Current elevation %.3f, target elevation %.3f", this->player_plane->elevationf / 10.0f,
                target_elevation);
}
void DebugStrike::radar() {
    // Get the canvas parameters for converting world-coordinates to canvas coordinates.
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();
    static float zoom = 1.0f;
    static ImVec2 pan_offset = ImVec2(0, 0);
    ImGuiIO& io = ImGui::GetIO();

    // Update zoom factor based on mouse wheel and adjust pan so that the zoom centers on the mouse.

    if (ImGui::IsWindowFocused() && io.MouseWheel != 0.0f) {
        float prev_zoom = zoom;
        zoom += io.MouseWheel * 0.1f;
        if (zoom < 0.1f)
            zoom = 0.1f;
        ImVec2 mouse_pos = io.MousePos;
        pan_offset.x = pan_offset.x * (zoom / prev_zoom) + (mouse_pos.x - canvas_pos.x) * (1 - (zoom / prev_zoom));
        pan_offset.y = pan_offset.y * (zoom / prev_zoom) + (mouse_pos.y - canvas_pos.y) * (1 - (zoom / prev_zoom));
    }

    // Update panning based on right mouse button drag.
    if (ImGui::IsWindowFocused() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        pan_offset.x += io.MouseDelta.x;
        pan_offset.y += io.MouseDelta.y;
    }

    // Adjust the canvas position and size using the pan and zoom.
    canvas_pos.x += pan_offset.x;
    canvas_pos.y += pan_offset.y;
    canvas_size.x *= zoom;
    canvas_size.y *= zoom;
    // Set a scale factor to map actor positions to the child window size.
    // You may need to adjust this value depending on your world-to-canvas mapping.

    float scale_x = canvas_size.x / (BLOCK_WIDTH * 18.0f);
    float scale_z = canvas_size.y / (BLOCK_WIDTH * 18.0f);

    // Draw a rectangle to represent the canvas area.
    draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(6, 57, 112, 255));
    // Optionally, center the drawing within the canvas.
    ImVec2 canvas_center = ImVec2(canvas_pos.x + canvas_size.x * 0.5f,
                                canvas_pos.y + canvas_size.y * 0.5f);
    // Draw a grid for better visualization.
    for (int i = -9; i <= 9; ++i) {
        // Vertical lines
        ImVec2 line_start = ImVec2(canvas_center.x + i * BLOCK_WIDTH * scale_x, canvas_pos.y);
        ImVec2 line_end = ImVec2(line_start.x, canvas_pos.y + canvas_size.y);
        draw_list->AddLine(line_start, line_end, IM_COL32(200, 200, 200, 255));

        // Horizontal lines
        line_start = ImVec2(canvas_pos.x, canvas_center.y + i * BLOCK_WIDTH * scale_z);
        line_end = ImVec2(canvas_pos.x + canvas_size.x, line_start.y);
        draw_list->AddLine(line_start, line_end, IM_COL32(200, 200, 200, 255));
    }
    for (auto area : this->current_mission->mission->mission_data.areas) {
        if (area) {
            // Map world coordinates (using x and z for a top-down view) to canvas coordinates.
            float world_x = area->position.x;
            float world_z = -area->position.z;
            ImVec2 area_canvas_pos = ImVec2(canvas_center.x + world_x * scale_x,
                                            canvas_center.y - world_z * scale_z);
            
            
            // Draw a square centered at the area's computed canvas position.
            ImVec2 top_left = ImVec2(area_canvas_pos.x - area->AreaWidth*scale_x *0.5f,
                                    area_canvas_pos.y - area->AreaWidth*scale_z * 0.5f);
            ImVec2 bottom_right = ImVec2(area_canvas_pos.x + area->AreaWidth*scale_x * 0.5f,
                                        area_canvas_pos.y + area->AreaWidth*scale_z * 0.5f);
            
            draw_list->AddRect(top_left, bottom_right, IM_COL32(0, 255, 0, 255));
            
            // Optionally, display the area id near the square.
            draw_list->AddText(ImVec2(bottom_right.x + 2, top_left.y), IM_COL32(255, 255, 255, 255),
                                area->AreaName);
        }
    }
    for (auto actor : this->current_mission->actors) {
        if (actor && actor->plane) {
            // Map world coordinates (using x and z for a top-down view) to canvas coordinates.
            float world_x = actor->plane->x;
            float world_z = -actor->plane->z;
            ImVec2 actor_canvas_pos = ImVec2(canvas_center.x + world_x * scale_x,
                                            canvas_center.y - world_z * scale_z);
            
            // Define square size.
            float square_size = 10.0f;
            
            // Draw a square centered at the actor's computed canvas position.
            ImVec2 top_left = ImVec2(actor_canvas_pos.x - square_size * 0.5f,
                                    actor_canvas_pos.y - square_size * 0.5f);
            ImVec2 bottom_right = ImVec2(actor_canvas_pos.x + square_size * 0.5f,
                                        actor_canvas_pos.y + square_size * 0.5f);
            
            draw_list->AddRect(top_left, bottom_right, IM_COL32(255, 0, 0, 255));
            for (auto wp: actor->plane->weaps_object) {
                if (wp) {
                    // Map world coordinates (using x and z for a top-down view) to canvas coordinates.
                    float wp_world_x = wp->x;
                    float wp_world_z = -wp->z;
                    ImVec2 wp_canvas_pos = ImVec2(canvas_center.x + wp_world_x * scale_x,
                                                canvas_center.y - wp_world_z * scale_z);
                    
                    // Draw a square centered at the weapon's computed canvas position.
                    ImVec2 wp_top_left = ImVec2(wp_canvas_pos.x - 5.0f * 0.5f,
                                                wp_canvas_pos.y - 5.0f * 0.5f);
                    ImVec2 wp_bottom_right = ImVec2(wp_canvas_pos.x + 5.0f * 0.5f,
                                                    wp_canvas_pos.y + 5.0f * 0.5f);
                    
                    draw_list->AddRect(wp_top_left, wp_bottom_right, IM_COL32(255, 255, 0, 255));
                }
            }
            // Optionally, display the actor id near the square.
            draw_list->AddText(ImVec2(bottom_right.x + 2, top_left.y),  IM_COL32(255, 255, 255, 255),
                                actor->actor_name.c_str());
            // if clicked, open a window with actor details
            if (ImGui::IsMouseHoveringRect(top_left, bottom_right)) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    this->target = actor;
                }
                ImGui::OpenPopup("Actor Details");
                if (ImGui::BeginPopup("Actor Details")) {
                    ImGui::Text("Actor: %s", actor->actor_name.c_str());
                    if (actor->plane) {
                        ImGui::Text("Air Speed: %d", actor->plane->airspeed);
                        ImGui::Text("Altitude: %.0f", actor->plane->y);
                        ImGui::Text("Heading: %.0f", 360 - (actor->plane->azimuthf / 10.0f));
                        ImGui::Text("Position: (%.2f, %.2f, %.2f)", actor->plane->x, actor->plane->y, actor->plane->z);
                    } else {
                        ImGui::Text("Position: (%.2f, %.2f, %.2f)", actor->object->position.x, actor->object->position.y, actor->object->position.z);
                    }
                    
                    ImGui::EndPopup();
                }
            }
        } else if (actor) {
            // Map world coordinates (using x and z for a top-down view) to canvas coordinates.
            float world_x = actor->object->position.x;
            float world_z = -actor->object->position.z;
            ImVec2 actor_canvas_pos = ImVec2(canvas_center.x + world_x * scale_x,
                                            canvas_center.y - world_z * scale_z);
            
            // Define square size.
            float square_size = 10.0f;
            
            // Draw a square centered at the actor's computed canvas position.
            ImVec2 top_left = ImVec2(actor_canvas_pos.x - square_size * 0.5f,
                                    actor_canvas_pos.y - square_size * 0.5f);
            ImVec2 bottom_right = ImVec2(actor_canvas_pos.x + square_size * 0.5f,
                                        actor_canvas_pos.y + square_size * 0.5f);
            ImVec2 circle_center = ImVec2(actor_canvas_pos.x, actor_canvas_pos.y);
            draw_list->AddCircle(circle_center, square_size, IM_COL32(255, 255, 0, 255));
            draw_list->AddRect(top_left, bottom_right, IM_COL32(255, 255, 0, 255));
            if (ImGui::IsMouseHoveringRect(top_left, bottom_right)) {
                ImGui::OpenPopup("Actor Details");
                if (ImGui::BeginPopup("Actor Details")) {
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        this->target = actor;
                    }
                    ImGui::Text("Actor: %s", actor->actor_name.c_str());
                    ImGui::Text("Position: (%.2f, %.2f, %.2f)", actor->object->position.x, actor->object->position.y, actor->object->position.z);
                    ImGui::EndPopup();
                }
            }
            // Optionally, display the actor id near the square.
            draw_list->AddText(ImVec2(bottom_right.x + 2, top_left.y),  IM_COL32(255, 255, 255, 255),
                                actor->actor_name.c_str());
        }
    }
}
void DebugStrike::showActorDetails(SCMissionActors* actor) {
    if (actor->profile != nullptr) {
        ImGui::Text("Name %s", actor->profile->radi.info.name.c_str());
        ImGui::Text("CallSign %s", actor->profile->radi.info.callsign.c_str());
        ImGui::Text("Is AI %d", actor->profile->ai.isAI);
        if (ImGui::TreeNode("MSGS")) {
            for (auto msg : actor->profile->radi.msgs) {
                ImGui::Text("- [%d]: %s", msg.first, msg.second.c_str());
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("ASKS")) {
            for (auto msg : actor->profile->radi.asks) {
                ImGui::Text("- [%s]: %s", msg.first.c_str(), msg.second.c_str());
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("AI MVRS")) {
            for (auto ai : actor->profile->ai.mvrs) {
                ImGui::Text("NODE [%d] - [%d]", ai.node_id, ai.value);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("AI GOAL")) {
            for (auto goal : actor->profile->ai.goal) {
                ImGui::Text("[%d]", goal);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("AI ATRB")) {
            ImGui::Text("TH %d", actor->profile->ai.atrb.TH);
            ImGui::Text("CN %d", actor->profile->ai.atrb.CN);
            ImGui::Text("VB %d", actor->profile->ai.atrb.VB);
            ImGui::Text("LY %d", actor->profile->ai.atrb.LY);
            ImGui::Text("FL %d", actor->profile->ai.atrb.FL);
            ImGui::Text("AG %d", actor->profile->ai.atrb.AG);
            ImGui::Text("AA %d", actor->profile->ai.atrb.AA);
            ImGui::Text("SM %d", actor->profile->ai.atrb.SM);
            ImGui::Text("AR %d", actor->profile->ai.atrb.AR);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Score")) {
            ImGui::Text("Score %d", actor->score);
            ImGui::Text("Plane %d", actor->plane_down);
            ImGui::Text("Ground %d", actor->ground_down);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("Name %s", actor->actor_name.c_str());
        ImGui::Text("No profile");
    }
    if (ImGui::TreeNode("On is activated")) {
        int cpt=0;
        if (actor->on_is_activated.size() > 0) {
            if (ImGui::TreeNode((void *)(intptr_t)&actor->on_is_activated, "Prog %d", cpt)) {
                for (auto opcodes: actor->on_is_activated) {
                    ImGui::Text("OPCODE [%d] %s\t\tARG [%d]", opcodes.opcode, prog_op_names[prog_op(opcodes.opcode)].c_str(), opcodes.arg);
                }
                ImGui::TreePop();
            }
            cpt++;
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("On update")) {
        int cpt=0;
        if (actor->on_update.size() > 0) {
            if (ImGui::TreeNode((void *)(intptr_t)&actor->on_update, "Prog %d", cpt)) {
                static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                if (ImGui::BeginTable("Flags", 2, flags)) {    
                    auto it = GameState.requierd_flags.begin();
                    int cpt = 0;
                    for (auto op : actor->on_update) {
                        ImGui::TableNextRow();
                        bool executed = false;
                        for (auto opt_traced: actor->executed_opcodes) {
                            if (opt_traced == cpt) {
                                executed = true;
                                break;
                            }
                        }
                        ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                        if (executed) {
                            cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.65f));
                        } else {
                            cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                        }
                        
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                        ImGui::Text("opcode:[%03d] %s ", op.opcode, prog_op_names[prog_op(op.opcode)].c_str());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                        ImGui::Text("arg:[%d]", op.arg);
                        cpt++;
                    }
                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
            cpt++;
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("On is destroyed")) {
        int cpt=0;
        if (actor->on_is_destroyed.size() > 0) {
            if (ImGui::TreeNode((void *)(intptr_t)&actor->on_is_destroyed, "Prog %d", cpt)) {
                for (auto opcodes: actor->on_is_destroyed) {
                    ImGui::Text("OPCODE [%d]\t\tARG [%d]", opcodes.opcode, opcodes.arg);
                }
                ImGui::TreePop();
            }
            cpt++;
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("On mission start")) {
        int cpt=0;
        if (actor->on_mission_start.size() > 0) {
            if (ImGui::TreeNode((void *)(intptr_t)&actor->on_is_destroyed, "Prog %d", cpt)) {
                for (auto opcodes: actor->on_mission_start) {
                    ImGui::Text("OPCODE [%d]\t\tARG [%d]", opcodes.opcode, opcodes.arg);
                }
                ImGui::TreePop();
            }
            cpt++;
        }
        ImGui::TreePop();
    }
}
void DebugStrike::renderMenu() {
    static bool show_simulation = false;
    static bool show_camera = false;
    static bool show_cockpit = false;
    static bool show_mission = false;
    static bool show_mission_parts_and_areas = false;
    static bool show_simulation_config = false;
    
    static bool show_music_player = false;
    static bool show_ai = false;
    
    static bool show_offcam = false;
    static bool show_load_plane = false;
    static bool show_radar = false;
    

    
    if (ImGui::BeginMenu("SCStrike")) {
        ImGui::MenuItem("Mission", NULL, &show_mission);
        ImGui::MenuItem("Mission part and Area", NULL, &show_mission_parts_and_areas);
        ImGui::MenuItem("Music", NULL, &show_music_player);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Debug")) {
        ImGui::MenuItem("Simulation", NULL, &show_simulation);
        ImGui::MenuItem("Simulation config", NULL, &show_simulation_config);
        ImGui::MenuItem("Camera", NULL, &show_camera);
        ImGui::MenuItem("Cockpit", NULL, &show_cockpit);
        ImGui::MenuItem("Show BBox", NULL, &this->show_bbox);
        ImGui::MenuItem("Ai pilot", NULL, &show_ai);
        ImGui::MenuItem("Off camera", NULL, &show_offcam);
        ImGui::MenuItem("Load plane", NULL, &show_load_plane);
        ImGui::MenuItem("Radar", NULL, &show_radar);
        ImGui::EndMenu();
    }
    int sceneid = -1;
    Vector3D plane_player_pos = {this->player_plane->x, this->player_plane->y, this->player_plane->z};
    uint8_t area_id = this->current_mission->getAreaID(plane_player_pos);
    ImGui::Text("Speed %d\tAltitude %.0f\tHeading %.0f\tTPS: %03d\tArea %s\tfilename: %s\tArea ID: %d",
                this->player_plane->airspeed, this->new_position.y,
                360 - (this->player_plane->azimuthf / 10.0f), this->player_plane->tps,
                this->current_mission->mission->mission_data.name.c_str(), this->miss_file_name.c_str(),area_id);

    if (show_offcam) {
        ImGui::Begin("Offcam");
        showOffCamera();
        ImGui::End();
    }
    if (show_load_plane) {
        ImGui::Begin("Load plane");
        loadPlane();
        ImGui::End();
    }
    if (show_radar) {
        if (ImGui::Begin("Actors Position")) {
            radar();
            ImGui::End();
        }
    
    }
    if (show_ai) {
        ImGui::Begin("AI");
        for (auto ai_actor: this->current_mission->actors) {
            if (ai_actor->pilot != nullptr) {
                ImGui::Text("AI %s", ai_actor->object->member_name.c_str());
                ImGui::SameLine();
                ImGui::Text("Speed %d\tAltitude %.0f\tHeading %.0f", ai_actor->plane->airspeed, ai_actor->plane->y,
                            360 - (ai_actor->plane->azimuthf / 10.0f));
                ImGui::SameLine();
                if (ai_actor->pilot != nullptr) {
                    ImGui::Text("{TH %.0f TA %d}", ai_actor->pilot->target_azimut, ai_actor->pilot->target_climb);
                    ImGui::SameLine();
                    ImGui::Text("AI OBJ %d", ai_actor->current_objective);
                    ImGui::SameLine();
                    ImGui::Text("AI Target %d", ai_actor->current_target);
                }
                ImGui::SameLine();
                ImGui::Text("X %.0f Y %.0f Z %.0f", ai_actor->plane->x, ai_actor->plane->y, ai_actor->plane->z);
                ImGui::SameLine();
                ImGui::Text("CX:[%d] CY:[%d]",ai_actor->plane->control_stick_x,ai_actor->plane->control_stick_y);
            }
        }
        ImGui::End();
    }
    if (show_music_player) {
        ImGui::Begin("Music Player");
        if (ImGui::BeginCombo("Music list", 0, 0)) {
            for (int i = 0; i < Mixer.music->musics[2].size(); i++) {
                if (ImGui::Selectable(std::to_string(i).c_str(), false))
                    Mixer.PlayMusic(i);
            }
            ImGui::EndCombo();
        }
        ImGui::End();
    }
    if (show_simulation_config) {
        ImGui::Begin("Simulation Config");
        simConfig();
        ImGui::End();
    }
    if (show_simulation) {
        ImGui::Begin("Simulation");
        simInfo();
        ImGui::End();
    }
    if (show_camera) {
        ImGui::Begin("Camera");
        ImGui::Text("Tps %d", this->player_plane->tps);
        ImGui::Text("Camera mode %d", this->camera_mode);
        ImGui::Text("Position [%.3f,%.3f,%.3f]", this->camera_pos.x, this->camera_pos.y, this->camera_pos.z);
        ImGui::Text("Pilot lookat [%d,%d]", this->pilote_lookat.x, this->pilote_lookat.y);
        ImGui::End();
    }
    if (show_cockpit) {
        ImGui::Begin("Cockpit");
        ImGui::Text("Throttle %d", this->player_plane->GetThrottle());

        ImGui::SameLine();
        ImGui::PushID(1);
        if (this->player_plane->GetWheel()) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Gears")) {
            this->player_plane->SetWheel();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID(1);
        if (this->player_plane->GetFlaps()) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Flaps")) {
            this->player_plane->SetFlaps();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID(1);
        if (this->player_plane->GetSpoilers()) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor::HSV(120.0f / 355.0f, 100.0f / 100.0f, 60.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.2f));
        }
        if (ImGui::Button("Breaks")) {
            this->player_plane->SetSpoilers();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        if (this->mouse_control) {
            ImGui::SameLine();
            ImGui::Text("Mouse control enabled");
        }
        ImGui::Text("Target %s", this->current_mission->actors[this->current_target]->actor_name.c_str());
        Vector3D targetPos = {this->target->object->position.x, this->target->object->position.y, this->target->object->position.z};
        Vector3D playerPos = {this->new_position.x, this->new_position.y, this->new_position.z};

        Camera totarget;
        totarget.SetPosition(&playerPos);
        totarget.LookAt(&targetPos);
        float raw;
        float yawn;
        float pitch;
        raw = 0.0f;
        yawn = 0.0f;
        pitch = 0.0f;

        totarget.GetOrientation().GetAngles(pitch, yawn, raw);

        Vector3D directionVector = {targetPos.x - playerPos.x, targetPos.y - playerPos.y, targetPos.z - playerPos.z};

        ImGui::Text("Target position [%.3f,%.3f,%.3f]", targetPos.x, targetPos.y, targetPos.z);
        ImGui::Text("Player position [%.3f,%.3f,%.3f]", playerPos.x, playerPos.y, playerPos.z);
        ImGui::Text("Azimuth to target %.3f",
                    (360 - (this->player_plane->azimuthf / 10.0f)) -
                        (atan2(targetPos.z - playerPos.z, targetPos.x - playerPos.x) * (180.0 / M_PI) + 90.0f));
        ImGui::Text("Q angles [%.3f,%.3f,%.3f] rel %.3f", pitch * 180.0 / M_PI, yawn * 180.0 / M_PI,
                    180.0f - (raw * 180.0 / M_PI),
                    (180.0f - (raw * 180.0 / M_PI)) - (360 - (this->player_plane->azimuthf / 10.0f)));
        if (ImGui::TreeNode("missiles")) {
            for (auto missils: this->player_plane->weaps_object) {
                ImGui::Text("VX:%.3f\tVY:%.3f\tVZ:%.3f\televation:%.3f\tazimut:%.3f\tspeed%.3f", 
                    missils->vx,
                    missils->vy,
                    missils->vz,
                    missils->elevationf,
                    missils->azimuthf,
                    Vector3D(missils->vx, missils->vy, missils->vz).Norm()
                );
                ImGui::Text("X:%.3f\tY:%.3f\tZ:%.3f", missils->x+ 180000.0f, missils->y, missils->z+ 180000.0f);
                if (missils->target != nullptr) {
                    ImGui::Text("Target X:%.0f\tY:%.0f\tZ:%.0f", missils->target->object->position.x, missils->target->object->position.y, missils->target->object->position.z);    
                }
                
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }
    if (show_mission) {
        ImGui::Begin("Mission");
        std::vector<const char *> to_mission_list= {
            "MISN-a1.IFF",
            "MISN-a2.IFF",
            "MISN-b1.IFF",
            "MISN-bw.IFF",
            "MISN-b2.IFF",
            "MISN-b3.IFF",
            "MISN-b4.IFF",
            "MISN-c1.IFF",
            "MISN-cw.IFF",
            "MISN-c2.IFF",
            "MISN-c3.IFF",
            "MISN-cy.IFF",
            "MISN-c4.IFF",
            "MISN-cz.IFF",
            "MISN-d1.IFF",
            "MISN-dw.IFF",
            "MISN-d2.IFF",
            "MISN-e1.IFF",
            "MISN-e2.IFF",
            "MISN-e3.IFF",
            "MISN-e4.IFF",
            "MISN-f1.IFF",
            "MISN-f2.IFF",
            "MISN-fx.IFF",
            "MISN-f3.IFF",
            "MISN-f4.IFF",
            "MISN-g1.IFF",
            "MISN-g2.IFF",
            "MISN-g3.IFF",
            "MISN-h1.IFF"
        };
        static ImGuiComboFlags flags = 0;
        static int to_mission_idx = 0;
        ImGui::PushItemWidth(200.0f);
        if (ImGui::BeginCombo("Strike Commander", mission_list[mission_idx], flags)) {
            for (int n = 0; n < SCSTRIKE_MAX_MISSIONS; n++) {
                const bool is_selected = (mission_idx == n);
                if (ImGui::Selectable(mission_list[n], is_selected))
                    mission_idx = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Load SC Mission")) {
            this->setMission((char *)mission_list[mission_idx]);
        }
        ImGui::PushItemWidth(200.0f);
        if (ImGui::BeginCombo("Tactical Operation", to_mission_list[to_mission_idx], flags)) {
            for (int n = 0; n < to_mission_list.size(); n++) {
                const bool is_selected = (to_mission_idx == n);
                if (ImGui::Selectable(to_mission_list[n], is_selected))
                    to_mission_idx = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Load TO Mission")) {
            this->setMission((char *)to_mission_list[to_mission_idx]);
        }

        ImGui::End();
    }

    if (show_mission_parts_and_areas) {
        ImGui::Begin("Mission Parts and Areas");
        ImGui::Text("Mission %s", this->current_mission->mission->mission_data.name.c_str());
        ImGui::Text("Area %s", this->current_mission->mission->mission_data.world_filename.c_str());
        ImGui::Text("Player Coord %.0f %.0f %.0f", this->current_mission->mission->getPlayerCoord()->position.x, this->current_mission->mission->getPlayerCoord()->position.y,
                    this->current_mission->mission->getPlayerCoord()->position.z);
        ImGui::Text("Messages %d", this->current_mission->mission->mission_data.messages.size());
        if (ImGui::TreeNode("Messages")) {
            for (auto msg : this->current_mission->mission->mission_data.messages) {
                ImGui::Text("- %s", msg->c_str());
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Areas")) {
            for (auto area : this->current_mission->mission->mission_data.areas) {
                if (ImGui::TreeNode((void *)(intptr_t)area->id, "Area id %d", area->id)) {
                    ImGui::Text("Area name %s", area->AreaName);
                    ImGui::Text("Area x %.0f y %.0f z %.0f", area->position.x, area->position.y, area->position.z);
                    ImGui::Text("Area width %d height %d", area->AreaWidth, area->AreaHeight);
                    for (auto ub: area->unknown_bytes) {
                        ImGui::Text("ub %d", ub);
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Area Objects")) {
            for (int g = 0; g < area.objects.size(); g++) {
                auto obj = &area.objects.at(g);
                if (ImGui::TreeNode((void *)(intptr_t)g, "Object id %d", g)) {
                    ImGui::Text("Object name %s", obj->name);
                    ImGui::Text("Object x %.3f y %.3f z %.3f", obj->position.x, obj->position.y, obj->position.z);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("SPOT")) {
            for (auto spot : this->current_mission->mission->mission_data.spots) {
                if (ImGui::TreeNode((void *)(intptr_t)spot, "Spot %d", spot->id)) {
                    ImGui::Text("Spot area_id %d", spot->area_id);
                    ImGui::Text("Spot x %.0f y %.0f z %.0f", spot->position.x, spot->position.y, spot->position.z);
                    ImGui::Text("Spot from file x %.0f y %.0f z %.0f", spot->origin.x, spot->origin.y, spot->origin.z);
                    ImGui::Text("u1 %d", spot->unknown1);
                    ImGui::Text("u2 %d", spot->unknown2);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("PROG")) {
            int id=1;
            for (auto prog : this->current_mission->mission->mission_data.prog) {
                if (ImGui::TreeNode((void *)(intptr_t)prog, "Prog %d", id)) {
                    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                    if (ImGui::BeginTable("Flags", 2, flags)) {    
                        auto it = GameState.requierd_flags.begin();
                        for (auto op : *prog) {
                            ImGui::TableNextRow();
                            /*ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                            if (it->second) {
                                cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.65f));
                            } else {
                                cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                            }
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);*/
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("opcode:[%03d] %s ", op.opcode, prog_op_names[prog_op(op.opcode)].c_str());
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("arg:[%d]", op.arg);
                        }
                        ImGui::EndTable();
                    }

                    ImGui::TreePop();
                }
                id++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Flags")) {
            int cpt_flags = 0;
            for (auto flag : this->current_mission->mission->mission_data.flags) {
                ImGui::Text("Flag %d : %d", cpt_flags, flag);
                cpt_flags++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Scenes")) {
            int cpt_scenes = 0;
            for (auto scene : this->current_mission->mission->mission_data.scenes) {
                if (ImGui::TreeNode((void *)(intptr_t)scene, "Scene %d", cpt_scenes)) {
                    if (scene->area_id >= 0) {
                        ImGui::Text("Scene area_id %s", this->current_mission->mission->mission_data.areas[scene->area_id]->AreaName);
                    } else {
                        ImGui::Text("Scene area_id %d", scene->area_id);
                    }
                    ImGui::Text("IS ACTIVE %d", scene->is_active);
                    ImGui::Text("Is COORD RELATIVE TO AREA %d", scene->is_coord_on_area);
                    for (auto cast : scene->cast) {
                        ImGui::Text("Cast %d", cast);
                    }
                    for (auto prog : scene->progs_id) {
                        ImGui::Text("Prog %d", prog);
                    }
                    for (auto b: scene->unknown_bytes) {
                        ImGui::Text("ub %d", b);
                    }
                    ImGui::TreePop();
                }
                cpt_scenes++;
            }
            ImGui::TreePop();
        }
        ImGui::Text("Mission Parts %d", this->current_mission->mission->mission_data.parts.size());
        if (ImGui::TreeNode("Parts")) {
            for (auto part : this->current_mission->mission->mission_data.parts) {
                if (ImGui::TreeNode((void *)(intptr_t)part->id, "Parts id %d, area id %d", part->id, part->area_id)) {
                    ImGui::Text("u1 %d", part->unknown1);
                    ImGui::Text("u2 %d", part->unknown2);
                    ImGui::Text("x %.0f y %.0f z %.0f", part->position.x, part->position.y, part->position.z);
                    ImGui::Text("azymuth %d", part->azymuth);
                    ImGui::Text("u3 %d", part->unknown3);
                    ImGui::Text("Name %s", part->member_name.c_str());
                    ImGui::Text("Destroyed %s", part->member_name_destroyed.c_str());
                    for (auto obj : part->unknown_bytes) {
                        ImGui::Text("ub %d", obj);
                    }
                    ImGui::Text("Weapon load %s", part->weapon_load.c_str());
                    if (ImGui::TreeNode("RS ENTITY")) {
                        ImGui::Text("weight %d", part->entity->weight_in_kg);
                        ImGui::Text("Thrust %d", part->entity->thrust_in_newton);
                        if (ImGui::TreeNode("childs")) {
                            for (auto child : part->entity->chld) {
                                ImGui::Text("%s - %d %d %d",child->name.c_str(), child->x, child->y, child->z);
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("weapons")) {
                            for (auto weapon : part->entity->weaps) {
                                ImGui::Text("%s - %d", weapon->name.c_str(), weapon->nb_weap);
                                if (weapon->objct != nullptr) {
                                    if (ImGui::TreeNode("weapons")) {
                                        if (weapon->objct->wdat != nullptr) {
                                            ImGui::Text("Weapon DATA %d, %d, %d, %d, %d", 
                                                weapon->objct->wdat->damage,
                                                weapon->objct->wdat->effective_range,
                                                weapon->objct->wdat->target_range,
                                                weapon->objct->wdat->tracking_cone,
                                                weapon->objct->wdat->radius
                                            );
                                        }
                                        if (weapon->objct->dynn_miss != nullptr) {
                                            ImGui::Text("Dynamics: %d, %d, %d, %d",
                                                weapon->objct->dynn_miss->velovity_m_per_sec,
                                                weapon->objct->dynn_miss->turn_degre_per_sec,
                                                weapon->objct->dynn_miss->proximity_cm,
                                                weapon->objct->weight_in_kg
                                            ); 
                                        }
                                        ImGui::TreePop();
                                    }
                                }
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("hpts")) {
                            for (auto hpt : part->entity->hpts) {
                                ImGui::Text("%d - %d %d %d",hpt->id,  hpt->x, hpt->y, hpt->z);
                            }
                            ImGui::TreePop();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Mission Actors")) {
            for (auto actor : current_mission->actors) {
                if (ImGui::TreeNode((void *)(intptr_t)actor, "Actor %d", actor->actor_id)) {
                    showActorDetails(actor);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Casting")) {
            for (auto cast : this->current_mission->mission->mission_data.casting) {
                if (ImGui::TreeNode((void *)(intptr_t)cast, "Actor %s", cast->actor.c_str())) {
                    if (cast->profile != nullptr) {
                        ImGui::Text("Name %s", cast->profile->radi.info.name.c_str());
                        ImGui::Text("CallSign %s", cast->profile->radi.info.callsign.c_str());
                        ImGui::Text("Is AI %d", cast->profile->ai.isAI);
                        if (ImGui::TreeNode("MSGS")) {
                            for (auto msg : cast->profile->radi.msgs) {
                                ImGui::Text("- [%d]: %s", msg.first, msg.second.c_str());
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("ASKS")) {
                            for (auto msg : cast->profile->radi.asks) {
                                ImGui::Text("- [%s]: %s", msg.first.c_str(), msg.second.c_str());
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNode("ASKS_VECTOR")) {
                            for (auto msg : cast->profile->radi.asks_vector) {
                                ImGui::Text("- %s", msg.c_str());
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        ImGui::End();
    }
}

void DebugStrike::renderUI() {
    if (ImGui::BeginTabBar("DebugTabs")) {
        if (ImGui::BeginTabItem("Mission Info")) {
            ImGui::BeginChild("Radar", ImVec2(0, 400), true);
            radar();
            ImGui::EndChild();
            ImGui::BeginChild("View", ImVec2(0, 200), true);
            ImGui::BeginChild("Camera", ImVec2(250, 0), true);
                showOffCamera();    
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("Target", ImVec2(0, 0), true);
                if (this->target != nullptr) {
                    ImGui::Text("Target: %s", this->target->actor_name.c_str());
                    if (this->target->plane) {
                        ImGui::Text("Air Speed: %d", this->target->plane->airspeed);
                        ImGui::Text("Altitude: %.0f", this->target->plane->y);
                        ImGui::Text("Heading: %.0f", 360 - (this->target->plane->azimuthf / 10.0f));
                        ImGui::Text("Position: (%.2f, %.2f, %.2f)", this->target->plane->x, this->target->plane->y, this->target->plane->z);
                    } else {
                        ImGui::Text("Position: (%.2f, %.2f, %.2f)", this->target->object->position.x, this->target->object->position.y, this->target->object->position.z);
                    }
                    if (this->target->pilot != nullptr) {
                        ImGui::Text("{TH %.0f TA %d}", this->target->pilot->target_azimut, this->target->pilot->target_climb);
                        
                        ImGui::Text("AI OBJ %d", this->target->current_objective);
                        
                        ImGui::Text("AI Target %d", this->target->current_target);
                    }
                }
            ImGui::EndChild();
            ImGui::EndChild();
            ImGui::BeginChild("Target info", ImVec2(0, 0), true);
            if (this->target != nullptr) {
                showActorDetails(this->target);
            } else {
                ImGui::Text("No target selected");
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Simulation")) {
            ImGui::BeginChild("Simulation Info", ImVec2(0, 370), true);
            simInfo();
            ImGui::EndChild();
            ImGui::BeginChild("Simulation config", ImVec2(0, 250), true);
            simConfig();
            ImGui::EndChild();
            ImGui::BeginChild("Planes", ImVec2(0, 0), true);
            loadPlane();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        // Additional tabs can be added here.
        ImGui::EndTabBar();
    }
    
}