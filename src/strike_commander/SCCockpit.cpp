//
//  SCCockpit.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 02/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include "precomp.h"
#include "SCCockpit.h"

Point2D rotateAroundPoint(Vector2D point, Point2D center, float angle) {
    float x    = point.x - center.x;
    float y    = point.y - center.y;
    float newx = x * cos(angle) - y * sin(angle) + center.x;
    float newy = x * sin(angle) + y * cos(angle) + center.y;
    return {(int)newx, (int)newy};
}
Vector2D rotateAroundPoint(Vector2D point, Vector2D center, float angle) {
    float x    = point.x - center.x;
    float y    = point.y - center.y;
    float newx = x * cos(angle) - y * sin(angle) + center.x;
    float newy = x * sin(angle) + y * cos(angle) + center.y;
    return {newx, newy};
}
Point2D rotateAroundPoint(Point2D point, Point2D center, float angle) {
    float x    = (float)(point.x - center.x);
    float y    = (float)(point.y - center.y);
    float newx = x * cos(angle) - y * sin(angle) + center.x;
    float newy = x * sin(angle) + y * cos(angle) + center.y;
    return {(int)newx, (int)newy};
}
Point2D rotate(Vector2D point, float angle) {
    float newx = point.x * cos(angle) - point.y * sin(angle);
    float newy = point.x * sin(angle) + point.y * cos(angle);
    return {(int)newx, (int)newy};
}
Point2D rotate(Point2D point, float angle) {
    float newx = point.x * cos(angle) - point.y * sin(angle);
    float newy = point.x * sin(angle) + point.y * cos(angle);
    return {(int)newx, (int)newy};
}
SCCockpit::SCCockpit() {}
SCCockpit::~SCCockpit() {
    if (this->hud_framebuffer) {
        delete this->hud_framebuffer;
        this->hud_framebuffer = nullptr;
    }
    if (this->mfd_right_framebuffer) {
        delete this->mfd_right_framebuffer;
        this->mfd_right_framebuffer = nullptr;
    }
    if (this->mfd_left_framebuffer) {
        delete this->mfd_left_framebuffer;
        this->mfd_left_framebuffer = nullptr;
    }
}
/**
 * SCCockpit::init
 *
 * Initialize the cockpit object from the standard SC cockpit assets.
 *
 * This function reads the palette from the standard SC palette IFF file,
 * initializes an RSPalette object from it, and assigns the result to the
 * SCCockpit::palette member variable.
 *
 * It also initializes the cockpit object from the F16 cockpit IFF file
 * and assigns the result to the SCCockpit::cockpit member variable.
 */
void SCCockpit::init() {
    RSPalette palette;
    palette.initFromFileData(Assets.GetFileData("PALETTE.IFF"));
    this->palette         = *palette.GetColorPalette();
    cockpit               = new RSCockpit(&Assets);
    TreEntry *cockpit_def = Assets.GetEntryByName("..\\..\\DATA\\OBJECTS\\F16-CKPT.IFF");
    cockpit->InitFromRam(cockpit_def->data, cockpit_def->size);

    TreEntry *hud_def = Assets.GetEntryByName("..\\..\\DATA\\OBJECTS\\HUD.IFF");
    hud               = new RSHud();
    hud->InitFromRam(hud_def->data, hud_def->size);
    for (int i = 0; i < 36; i++) {
        HudLine line;
        line.start.x = 0;
        line.start.y = 0 + i * 20;
        line.end.x   = 70;
        line.end.y   = 0 + i * 20;
        horizon.push_back(line);
    }
    hud_framebuffer       = new FrameBuffer(96, 93);
    mfd_right_framebuffer = new FrameBuffer(115, 95);
    mfd_left_framebuffer  = new FrameBuffer(115, 95);
    raws_framebuffer      = new FrameBuffer(36, 32);
    target_framebuffer    = new FrameBuffer(320, 200);
    alti_framebuffer      = new FrameBuffer(33, 29);
    speed_framebuffer     = new FrameBuffer(36, 29);
    this->font            = FontManager.GetFont("..\\..\\DATA\\FONTS\\SHUDFONT.SHP");
    this->big_font        = FontManager.GetFont("..\\..\\DATA\\FONTS\\HUDFONT.SHP");
}
/**
 * SCCockpit::RenderAltitude
 *
 * Render the current altitude in the cockpit view.
 *
 * This function renders the current altitude in the cockpit view by drawing
 * a band of lines and numbers on the right side of the screen. The altitude
 * is rendered in meters, with the 1000s place on the left side of the band
 * and the 100s place on the right side of the band. The band is rendered
 * from the top of the screen (0) to the bottom of the screen (512), with
 * the 0 mark at the top of the screen and the 1000 mark at the bottom of
 * the screen.
 */

void SCCockpit::RenderAltitude(Point2D alti_top_left = {185, 30}, FrameBuffer *fb = VGA.GetFrameBuffer()) {

    std::vector<Point2D> alti_band_roll;
    float alti_in_feet = this->altitude * 3.28084f; // Convert meters to feet
    Point2D alti_size  = {20, 35};

    Point2D bottom_right = {alti_top_left.x + alti_size.x, alti_top_left.y + alti_size.y};

    alti_band_roll.reserve(100);
    for (int i = 0; i < 100; i++) {
        Point2D p;
        p.x = 0;
        p.y = (100 - i) * 10;
        alti_band_roll.push_back(p);
    }
    int cpt = 1000;
    for (auto p : alti_band_roll) {
        Point2D p2 = {p.x, p.y};
        p2.x       = alti_top_left.x + 9;
        p2.y       = (alti_top_left.y + alti_size.y / 2) - p.y + (int)(alti_in_feet / 100);
        if (p2.y > alti_top_left.y && p2.y < bottom_right.y) {
            fb->PrintText(this->font, &p2, (char *)std::to_string(cpt / 10.0f).c_str(), 0, 0, 3, 2, 2);
        }
        cpt -= 10;
        Point2D alti = {alti_top_left.x + 1, p2.y};
        int sheight  = this->hud->small_hud->ALTI->SHAP->GetHeight();
        // alti.y = alti_top_left.y;
        this->hud->small_hud->ALTI->SHAP->SetPosition(&alti);
        fb->DrawShapeWithBox(
            this->hud->small_hud->ALTI->SHAP, alti_top_left.x, bottom_right.x, alti_top_left.y - 10, bottom_right.y - 10
        );
    }
    Point2D alti_arrow = {alti_top_left.x-3, alti_top_left.y + alti_size.y / 2};
    fb->line(
        alti_arrow.x, alti_arrow.y, alti_arrow.x + 1, alti_arrow.y, 223
    );
    alti_arrow.y -= this->hud->small_hud->ALTI->SHP2->GetHeight() / 2;
    this->hud->small_hud->ALTI->SHP2->SetPosition(&alti_arrow);
    fb->DrawShape(this->hud->small_hud->ALTI->SHP2);
    Point2D alti_text = {alti_top_left.x, alti_top_left.y + 5 + this->hud->small_hud->ALTI->SHAP->GetHeight()};
    fb->PrintText(this->font, &alti_text, (char *)std::to_string(alti_in_feet).c_str(), 0, 0, 5, 2, 2);

}
/**
 * Renders the heading indicator in the cockpit view.
 *
 * This function renders the heading indicator in the cockpit view by drawing
 * a band of lines and numbers on the top of the screen. The heading is rendered
 * in degrees, with the 0 mark at the top of the screen and the 360 mark at the
 * bottom of the screen. The band is rendered from the top of the screen (0) to
 * the bottom of the screen (360), with the 0 mark at the top of the screen and
 * the 360 mark at the bottom of the screen.
 */
void SCCockpit::RenderHeading(Point2D heading_pos = {136, 90}, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    this->hud->small_hud->HEAD->SHAP->SetPosition(&heading_pos);

    std::vector<Point2D> heading_points;
    std::string txt;
    heading_points.reserve(36);
    for (int i = 0; i < 36; i++) {
        Point2D p;
        p.x = 432 - (36 - i) * 12;
        p.y = heading_pos.y + 7;
        heading_points.push_back(p);
    }
    int headcpt  = 0;
    int pixelcpt = 0;

    Point2D heading_size = {48, 14};
    Point2D bottom_right = {heading_pos.x + heading_size.x, heading_pos.y + heading_size.y};

    for (auto p : heading_points) {
        Point2D hp = p;
        hp.x       = hp.x + (int)(this->heading * 1.2) + (heading_pos.x + heading_size.x / 2);

        if (hp.x < 0) {
            hp.x += 432;
        }
        if (hp.x > 432) {
            hp.x -= 432;
        }
        if (hp.x > heading_pos.x && hp.x < bottom_right.x) {
            Point2D txt_pos = hp;
            int toprint     = headcpt;
            if (toprint == 36) {
                toprint = 0;
            }
            txt = std::to_string(toprint);
            fb->PrintText(this->font, &txt_pos, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2);
        }
        if (headcpt % 3 == 0) {
            Point2D headspeed;
            headspeed.x = hp.x - 6;
            headspeed.y = heading_pos.y;
            this->hud->small_hud->HEAD->SHAP->SetPosition(&headspeed);
            fb->DrawShapeWithBox(
                this->hud->small_hud->HEAD->SHAP, heading_pos.x, bottom_right.x, heading_pos.y, bottom_right.y
            );
        }
        headcpt += 1;
    }
    Vector2D weapoint_direction = {
        this->weapoint_coords.x - this->player->position.x, this->weapoint_coords.y - this->player->position.z
    };
    float weapoint_azimut = (atan2f(weapoint_direction.y, weapoint_direction.x) * 180.0f / (float)M_PI);
    Point2D weapoint      = {0, heading_pos.y - 3};

    weapoint_azimut -= 360;
    weapoint_azimut += 90;
    if (weapoint_azimut > 360) {
        weapoint_azimut -= 360;
    }
    while (weapoint_azimut < 0) {
        weapoint_azimut += 360;
    }
    this->way_az = weapoint_azimut;
    weapoint.x   = weapoint.x + (int)(weapoint_azimut * 1.2f) - (int)((360 - this->heading) * 1.2f) +
                 (heading_pos.x + heading_size.x / 2);
    if (weapoint.x < 0) {
        weapoint.x += 432;
    }
    if (weapoint.x > 432) {
        weapoint.x -= 432;
    }

    this->hud->small_hud->HEAD->SHP2->SetPosition(&weapoint);
    fb->line(
        (heading_pos.x + heading_size.x / 2), weapoint.y, (heading_pos.x + heading_size.x / 2), weapoint.y + 3, 223
    );
    fb->DrawShapeWithBox(
        this->hud->small_hud->HEAD->SHP2, heading_pos.x, bottom_right.x, heading_pos.y - 5, bottom_right.y
    );
}
void SCCockpit::RenderSpeed(Point2D speed_top_left = {115, 30}, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    std::vector<Point2D> speed_band_roll;
    Point2D speed_band_size = {20, 35};
    Point2D bottom_right    = {speed_top_left.x + speed_band_size.x, speed_top_left.y + speed_band_size.y};

    std::string txt;
    speed_band_roll.reserve(30);
    for (int i = 0; i < 150; i++) {
        Point2D p;
        p.x = 0;
        p.y = (150 - i) * 10;
        speed_band_roll.push_back(p);
    }
    int cpt_speed = 1500;
    for (auto sp : speed_band_roll) {
        Point2D p = sp;
        p.x       = speed_top_left.x + 5;
        p.y       = (speed_top_left.y + this->hud->small_hud->ASPD->SHAP->GetHeight() / 2) - p.y + (int)this->speed;
        if (p.y > speed_top_left.y + 1 && p.y < speed_top_left.y + this->hud->small_hud->ASPD->SHAP->GetHeight()) {
            txt = std::to_string(cpt_speed);
            fb->PrintText(this->font, &p, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2);
        }
        if (cpt_speed % 5 == 0) {
            if (p.y > 10 && p.y < 29 + this->hud->small_hud->ASPD->SHAP->GetHeight()) {
                Point2D speed = {bottom_right.x - 5, speed_top_left.y};
                speed.y       = p.y;
                this->hud->small_hud->ASPD->SHAP->SetPosition(&speed);
                fb->DrawShapeWithBox(
                    this->hud->small_hud->ASPD->SHAP, speed_top_left.x, bottom_right.x, speed_top_left.y - 10,
                    bottom_right.y - 10
                );
            }
        }
        cpt_speed -= 10;
    }
    Point2D speed_arrow = {bottom_right.x, speed_top_left.y + speed_band_size.y / 2};
    fb->line(
        speed_arrow.x-3, speed_arrow.y, speed_arrow.x, speed_arrow.y, 223
    );
}
/**
 * SCCockpit::RenderHudHorizonLinesSmall
 *
 * Renders the horizon lines of the Heads-Up Display (HUD) in the lower
 * left corner of the screen. The horizon lines are used to indicate the
 * orientation of the aircraft with respect to the horizon.
 *
 * The horizon lines are drawn with a set of HudLine objects, which are
 * transformed by the roll angle of the aircraft. The lines are also
 * clipped to the bounds of the HUD display area.
 *
 * This function is called by the SCCockpit::Render method.
 */
void SCCockpit::RenderHudHorizonLinesSmall(Point2D center = {160, 50}, FrameBuffer *fb = VGA.GetFrameBuffer()) {

    const int dec = 0;

    const int width  = 50;
    const int height = 78;

    int bx1 = center.x - width / 2;
    int bx2 = center.x + width / 2;
    int by1 = center.y - height / 2;
    int by2 = center.y + height / 2;

    int top         = by1;
    int bottom      = by2;
    int left        = bx1 + 5;
    int ligne_width = width - 10;
    std::string txt;
    std::vector<HudLine> *hline = new std::vector<HudLine>();
    hline->resize(36);
    for (int i = 0; i < 36; i++) {
        hline->at(i).start.x = horizon[i].start.x;
        hline->at(i).start.y = (int)(horizon[i].start.y - ((360 - center.y) - this->pitch * 4)) % 720;
        if (hline->at(i).start.y < 0) {
            hline->at(i).start.y += 720;
        }
        hline->at(i).end.x = horizon[i].end.x;
        hline->at(i).end.y = hline->at(i).start.y;
    }

    int ladder = 90;
    for (auto h : *hline) {
        HudLine l  = h;
        HudLine l2 = h;
        l.start.x  = l.start.x + left;
        l.start.y  = l.start.y - dec;
        l.end.y    = l.end.y - dec;
        l.end.x    = l.start.x + ligne_width / 4;

        l2.start.x = l.start.x + ligne_width - ligne_width / 4;
        l2.start.y = l2.start.y - dec;
        l2.end.y   = l2.end.y - dec;
        l2.end.x   = l.start.x + ligne_width;

        l.start = rotateAroundPoint(l.start, center, this->roll * (float)M_PI / 180.0f);
        l.end   = rotateAroundPoint(l.end, center, this->roll * (float)M_PI / 180.0f);

        l2.start = rotateAroundPoint(l2.start, center, this->roll * (float)M_PI / 180.0f);
        l2.end   = rotateAroundPoint(l2.end, center, this->roll * (float)M_PI / 180.0f);
        txt      = std::to_string(ladder);
        if (l.start.x > bx1 && l.start.x < bx2 && l.start.y > by1 && l.start.y < by2) {
            Point2D p = l.start;
            p.y -= 2;
            fb->PrintText(this->font, &p, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2);
        }
        if (l2.end.x > bx1 && l2.end.x < bx2 && l2.end.y > by1 && l2.end.y < by2) {
            Point2D p = l2.end;
            p.x       = p.x - 8;
            p.y -= 2;
            fb->PrintText(this->font, &p, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2);
        }
        int color = 223;
        if (ladder == 0) {
            HudLine l3 = h;
            l3.start.x = l3.start.x + left - 5;
            l3.start.y = l3.start.y - dec;
            l3.end.y   = l3.end.y - dec;
            l3.end.x   = l3.start.x + ligne_width + 10;
            l3.start   = rotateAroundPoint(l3.start, center, this->roll * (float)M_PI / 180.0f);
            l3.end     = rotateAroundPoint(l3.end, center, this->roll * (float)M_PI / 180.0f);
            fb->lineWithBox(l3.start.x, l3.start.y, l3.end.x, l3.end.y, color, bx1, bx2, by1, by2);
        } else {
            int skip = 1;
            if (ladder < 0) {
                skip = 2;
            }
            fb->lineWithBoxWithSkip(l.start.x, l.start.y, l.end.x, l.end.y, color, bx1, bx2, by1, by2, skip);
            fb->lineWithBoxWithSkip(l2.start.x, l2.start.y, l2.end.x, l2.end.y, color, bx1, bx2, by1, by2, skip);
        }
        ladder = ladder - 5;
    }
}

void SCCockpit::RenderMFDS(Point2D mfds, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    this->cockpit->MONI.SHAP.SetPosition(&mfds);
    for (int i = 0; i < this->cockpit->MONI.SHAP.GetHeight(); i++) {
        fb->line(mfds.x, mfds.y + i, mfds.x + this->cockpit->MONI.SHAP.GetWidth(), mfds.y + i, 2);
    }
    fb->DrawShape(&this->cockpit->MONI.SHAP);
}
void SCCockpit::RenderTargetWithCam(Point2D top_left = {126, 5}, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    Point2D hud_size = {68, 85};
    Point2D bottom_right = {top_left.x + hud_size.x, top_left.y + hud_size.y};

    if (this->target != nullptr) {
        Vector3D campos       = this->cam->GetPosition();
        Vector3DHomogeneous v = {this->target->position.x, this->target->position.y, this->target->position.z, 1.0f};

        Matrix *mproj = this->cam->GetProjectionMatrix();
        Matrix *mview = this->cam->GetViewMatrix();

        Vector3DHomogeneous mcombined = mview->multiplyMatrixVector(v);
        Vector3DHomogeneous result    = mproj->multiplyMatrixVector(mcombined);
        if (result.z > 0.0f) {
            float x = result.x / result.w;
            float y = result.y / result.w;

            int Xhud = (int)((x + 1.0f) * 160.0f);
            int Yhud = (int)((1.0f - y - 0.45f) * 100.0f) - 1;
            
            // Calculate HUD coordinates relative to the HUD's frame of reference
            int hudX = Xhud - top_left.x;
            int hudY = Yhud - top_left.y;

            // Check if the target is within HUD boundaries
            bool isInHud = (Xhud >= top_left.x && Xhud < bottom_right.x && 
                            Yhud >= top_left.y && Yhud < bottom_right.y);

            isInHud = (hudX >= 0 && hudX < fb->width && 
                            hudX >= 0 && hudX < fb->height);
            // If the target is within the HUD boundaries, draw a targeting indicator
            if (isInHud) {
                // Draw a targeting reticle at the position
                Point2D targetPoint = {hudX+top_left.x, hudY+top_left.y};
                
                // Draw target reticle using HUD coordinates
                fb->lineWithBox(
                    targetPoint.x - 5, targetPoint.y - 5, targetPoint.x + 5, targetPoint.y - 5, 223, 
                    0, fb->width, 0, fb->height
                );
                fb->lineWithBox(
                    targetPoint.x + 5, targetPoint.y - 5, targetPoint.x + 5, targetPoint.y + 5, 223,
                    0, fb->width, 0, fb->height
                );
                fb->lineWithBox(
                    targetPoint.x + 5, targetPoint.y + 5, targetPoint.x - 5, targetPoint.y + 5, 223,
                    0, fb->width, 0, fb->height
                );
                fb->lineWithBox(
                    targetPoint.x - 5, targetPoint.y + 5, targetPoint.x - 5, targetPoint.y - 5, 223,
                    0, fb->width, 0, fb->height
                );
            }


            if (Xhud > 0 && Xhud < 320 && Yhud > 0 && Yhud < 200) {
                Point2D p = {Xhud, Yhud};
                fb->lineWithBox(
                    (int)p.x - 5, (int)p.y - 5, (int)p.x + 5, (int)p.y - 5, 223, top_left.x, bottom_right.x, top_left.y, bottom_right.y
                );
                fb->lineWithBox(
                    (int)p.x + 5, (int)p.y - 5, (int)p.x + 5, (int)p.y + 5, 223, top_left.x, bottom_right.x, top_left.y, bottom_right.y
                );
                fb->lineWithBox(
                    (int)p.x + 5, (int)p.y + 5, (int)p.x - 5, (int)p.y + 5, 223, top_left.x, bottom_right.x, top_left.y, bottom_right.y
                );
                fb->lineWithBox(
                    (int)p.x - 5, (int)p.y + 5, (int)p.x - 5, (int)p.y - 5, 223, top_left.x, bottom_right.x, top_left.y, bottom_right.y
                );
            }
        }
    }
}

void SCCockpit::RenderTargetingReticle() {
    GunSimulatedObject *weap = new GunSimulatedObject();
    Vector3D direction       = {
        this->player_plane->x - this->player_plane->last_px, this->player_plane->y - this->player_plane->last_py,
        this->player_plane->z - this->player_plane->last_pz
    };
    float planeSpeed      = direction.Length();
    float thrustMagnitude = -planeSpeed;
    thrustMagnitude       = -planeSpeed * 150.0f; // coefficient ajustable

    float yawRad   = tenthOfDegreeToRad(this->player_plane->yaw);
    float pitchRad = tenthOfDegreeToRad(-this->player_plane->pitch);
    float rollRad  = tenthOfDegreeToRad(0.0f);
    float cosRoll  = cosf(rollRad);
    float sinRoll  = sinf(rollRad);
    Vector3D initial_trust{0, 0, 0};
    initial_trust.x =
        thrustMagnitude * (cosf(pitchRad) * sinf(yawRad) * cosRoll + sinf(pitchRad) * cosf(yawRad) * sinRoll);
    initial_trust.y = thrustMagnitude * (sinf(pitchRad) * cosRoll - cosf(pitchRad) * sinf(yawRad) * sinRoll);
    initial_trust.z = thrustMagnitude * cosf(pitchRad) * cosf(yawRad);

    weap->obj       = this->player_plane->weaps_load[0]->objct;
    Vector3D campos = this->cam->GetPosition();
    weap->x         = this->player_plane->x;
    weap->y         = this->player_plane->y;
    weap->z         = this->player_plane->z;
    weap->vx        = initial_trust.x;
    weap->vy        = initial_trust.y;
    weap->vz        = initial_trust.z;

    weap->weight     = this->player_plane->weaps_load[0]->objct->weight_in_kg * 2.205f;
    weap->azimuthf   = this->player_plane->azimuthf;
    weap->elevationf = this->player_plane->elevationf;
    weap->target     = nullptr;
    Vector3D target{0, 0, 0};
    Vector3D velo{0, 0, 0};
    float yawRad_update   = yawRad;
    float pitchRad_update = pitchRad;
    float rollRad_update  = rollRad;

    for (int i = 0; i < 150; i++) {
        std::tie(target, velo) = weap->ComputeTrajectory(this->player_plane->tps);
        weap->x                = target.x + direction.x;
        weap->y                = target.y + direction.y;
        weap->z                = target.z + direction.z;

        weap->vx = velo.x;
        weap->vy = velo.y;
        weap->vz = velo.z;
    }

    Vector3DHomogeneous v = {target.x, target.y, target.z, 1.0f};

    Matrix *mproj = this->cam->GetProjectionMatrix();
    Matrix *mview = this->cam->GetViewMatrix();

    Vector3DHomogeneous mcombined = mview->multiplyMatrixVector(v);
    Vector3DHomogeneous result    = mproj->multiplyMatrixVector(mcombined);

    if (result.z > 0.0f) {
        float x = result.x / result.w;
        float y = result.y / result.w;

        int Xhud = (int)((x + 1.0f) * 160.0f);
        int Yhud = (int)((1.0f - y - 0.50f) * 100.0f) - 1;

        if (Xhud > 0 && Xhud < 320 && Yhud > 0 && Yhud < 200) {
            Point2D p = {Xhud, Yhud};
            VGA.GetFrameBuffer()->plot_pixel((int)p.x, (int)p.y, 223);
            VGA.GetFrameBuffer()->circle_slow((int)p.x, (int)p.y, 6, 90);
        }
    }
}
void SCCockpit::RenderBombSight() {
    const Point2D center{160, 50};
    const int width  = 50;
    const int height = 60;

    int bx1                  = center.x - width / 2;
    int bx2                  = center.x + width / 2;
    int by1                  = center.y - height / 2;
    int by2                  = center.y + height / 2;
    GunSimulatedObject *weap = new GunSimulatedObject();
    Vector3D direction       = {
        this->player_plane->x - this->player_plane->last_px, this->player_plane->y - this->player_plane->last_py,
        this->player_plane->z - this->player_plane->last_pz
    };
    float planeSpeed      = direction.Length();
    float thrustMagnitude = -planeSpeed;
    thrustMagnitude       = -planeSpeed * 50.0f; // coefficient ajustable

    float yawRad   = tenthOfDegreeToRad(this->player_plane->yaw);
    float pitchRad = tenthOfDegreeToRad(-this->player_plane->pitch);
    float rollRad  = tenthOfDegreeToRad(0.0f);
    float cosRoll  = cosf(rollRad);
    float sinRoll  = sinf(rollRad);
    Vector3D initial_trust{0, 0, 0};
    initial_trust.x = thrustMagnitude * (cosf(pitchRad) * sinf(yawRad) * cosRoll + sinf(pitchRad) * cosf(yawRad) * sinRoll);
    initial_trust.y = thrustMagnitude * (sinf(pitchRad) * cosRoll - cosf(pitchRad) * sinf(yawRad) * sinRoll);
    initial_trust.z = thrustMagnitude * cosf(pitchRad) * cosf(yawRad);

    weap->obj       = this->player_plane->weaps_load[this->player_plane->selected_weapon]->objct;
    Vector3D campos = this->cam->GetPosition();
    weap->x         = campos.x;
    weap->y         = campos.y;
    weap->z         = campos.z;
    weap->vx        = initial_trust.x;
    weap->vy        = initial_trust.y;
    weap->vz        = initial_trust.z;

    weap->weight   = this->player_plane->weaps_load[this->player_plane->selected_weapon]->objct->weight_in_kg * 2.205f;
    weap->azimuthf = this->player_plane->yaw;
    weap->elevationf = this->player_plane->pitch;
    weap->target     = nullptr;
    weap->mission    = this->current_mission;
    Vector3D target{0, 0, 0};
    Vector3D velo{0, 0, 0};
    std::tie(target, velo) = weap->ComputeTrajectoryUntilGround(this->player_plane->tps);
    weap->x                = target.x;
    weap->y                = target.y;
    weap->z                = target.z;
    weap->vx               = velo.x;
    weap->vy               = velo.y;
    weap->vz               = velo.z;
    Vector3DHomogeneous v = {target.x, target.y, target.z, 1.0f};

    Matrix *mproj = this->cam->GetProjectionMatrix();
    Matrix *mview = this->cam->GetViewMatrix();

    Vector3DHomogeneous mcombined = mview->multiplyMatrixVector(v);
    Vector3DHomogeneous result    = mproj->multiplyMatrixVector(mcombined);

    if (result.z > 0.0f) {
        float x = result.x / result.w;
        float y = result.y / result.w;

        int Xhud = (int)((x + 1.0f) * 160.0f);
        int Yhud = (int)((1.0f - y - 0.45f) * 100.0f) - 1;

        VGA.GetFrameBuffer()->plot_pixel(center.x, center.y, 223);
        VGA.GetFrameBuffer()->lineWithBox(center.x, center.y, Xhud, Yhud, 223, bx1, bx2, by1, by2);
        if (Xhud > 0 && Xhud < 320 && Yhud > 0 && Yhud < 200) {
            Point2D p = {Xhud, Yhud};
            VGA.GetFrameBuffer()->plot_pixel((int)p.x, (int)p.y, 223);
            VGA.GetFrameBuffer()->circle_slow((int)p.x, (int)p.y, 6, 90);
        }
    }
}
void SCCockpit::RenderMFDSWeapon(Point2D pmfd_right, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    std::string txt;
    this->RenderMFDS(pmfd_right, fb);
    Point2D pmfd_right_center = {
        pmfd_right.x + this->cockpit->MONI.SHAP.GetWidth() / 2, pmfd_right.y + this->cockpit->MONI.SHAP.GetHeight() / 2
    };
    Point2D pmfd_right_weapon = {
        pmfd_right_center.x - this->cockpit->MONI.MFDS.WEAP.ARTS.GetShape(0)->GetWidth() / 2,
        pmfd_right_center.y - 10 - this->cockpit->MONI.MFDS.WEAP.ARTS.GetShape(0)->GetHeight() / 2
    };
    this->cockpit->MONI.MFDS.WEAP.ARTS.GetShape(0)->SetPosition(&pmfd_right_weapon);
    fb->DrawShape(this->cockpit->MONI.MFDS.WEAP.ARTS.GetShape(0));
    std::map<int, int> weapons_shape = {
        {1,  1},
        {2,  1},
        {3,  5},
        {4,  7},
        {5,  9},
        {6, 11},
        {7, 13},
        {8, 15},
        {9,  3}
    };
    std::map<int, std::string> weapon_names = {
        {12,     "GUN"},
        { 1,  "AIM-9J"},
        { 2,  "AIM-9M"},
        { 3,  "AGM-65"},
        { 4,     "POD"},
        { 5,   "MK-20"},
        { 6,   "MK-82"},
        { 7,     "DUR"},
        { 8,  "GBU-15"},
        { 9, "AIM-120"}
    };
    if (this->player_plane->object->entity->hpts.size() == 9) {
        for (int indice = 1; indice < 5; indice++) {
            if (this->player_plane->weaps_load[indice] == nullptr) {
                continue;
            }
            int weapon_id    = this->player_plane->weaps_load[indice]->objct->wdat->weapon_id;
            int nb_weap      = this->player_plane->weaps_load[indice]->nb_weap;
            int i            = 4 - indice;
            int selected     = indice == this->player_plane->selected_weapon;
            RLEShape *shape  = this->cockpit->MONI.MFDS.WEAP.ARTS.GetShape(weapons_shape[weapon_id] + selected);
            int32_t s_width  = shape->GetWidth();
            int32_t s_height = shape->GetHeight();

            if (nb_weap > 0) {
                Point2D pmfd_right_weapon_hp = {
                    pmfd_right_center.x - 15 - s_width / 2 - i * 9, pmfd_right_center.y - 18 - s_height / 2 + i * 9
                };
                shape->SetPosition(&pmfd_right_weapon_hp);
                fb->DrawShape(shape);
                txt                               = std::to_string(nb_weap);
                Point2D pmfd_right_weapon_hp_text = {
                    pmfd_right_weapon_hp.x + s_width / 2 + 1, pmfd_right_weapon_hp.y + s_height + 6
                };
                fb->PrintText(
                    this->big_font, &pmfd_right_weapon_hp_text, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2
                );
            }
            if (this->player_plane->weaps_load[9 - indice] == nullptr) {
                continue;
            }
            nb_weap  = this->player_plane->weaps_load[9 - indice]->nb_weap;
            selected = 9 - indice == this->player_plane->selected_weapon;
            shape    = this->cockpit->MONI.MFDS.WEAP.ARTS.GetShape(weapons_shape[weapon_id] + selected);
            if (nb_weap > 0) {
                Point2D pmfd_right_weapon_hp_left = {
                    pmfd_right_center.x + 13 - s_width / 2 + i * 9, pmfd_right_center.y - 18 - s_height / 2 + i * 9
                };
                shape->SetPosition(&pmfd_right_weapon_hp_left);
                fb->DrawShape(shape);
                txt                                    = std::to_string(nb_weap);
                Point2D pmfd_right_weapon_hp_text_left = {
                    pmfd_right_weapon_hp_left.x + s_width / 2 - 1, pmfd_right_weapon_hp_left.y + s_height + 6
                };
                fb->PrintText(
                    this->big_font, &pmfd_right_weapon_hp_text_left, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(),
                    2, 2
                );
            }
        }
    }

    int sel_weapon_id = 0;
    int sel_nb_weap   = 0;
    if (this->player_plane->weaps_load[this->player_plane->selected_weapon] != nullptr) {
        sel_weapon_id = this->player_plane->weaps_load[this->player_plane->selected_weapon]->objct->wdat->weapon_id;
        sel_nb_weap   = this->player_plane->weaps_load[this->player_plane->selected_weapon]->nb_weap;
    }

    Point2D pmfd_right_weapon_gun{
        pmfd_right_weapon.x - 8 + this->cockpit->MONI.MFDS.WEAP.ARTS.GetShape(0)->GetWidth() / 2,
        pmfd_right_weapon.y + 6
    };
    txt = std::to_string(sel_nb_weap);
    fb->PrintText(this->big_font, &pmfd_right_weapon_gun, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2);

    Point2D pmfd_right_weapon_radar{pmfd_right_weapon.x, pmfd_right_weapon.y + 5};
    fb->PrintText(this->big_font, &pmfd_right_weapon_radar, const_cast<char *>("NORM"), 0, 0, 4, 2, 2);

    Point2D pmfd_right_weapon_selected{
        pmfd_right_weapon.x + 12 + this->cockpit->MONI.MFDS.WEAP.ARTS.GetShape(0)->GetWidth() / 2,
        pmfd_right_weapon.y + 5
    };
    fb->PrintText(
        this->big_font, &pmfd_right_weapon_selected, (char *)weapon_names[sel_weapon_id].c_str(), 0, 0,
        (uint32_t)weapon_names[sel_weapon_id].length(), 2, 2
    );

    Point2D pmfd_right_weapon_chaff{
        pmfd_right_weapon.x - 7 + this->cockpit->MONI.MFDS.WEAP.ARTS.GetShape(0)->GetWidth() / 2,
        pmfd_right_weapon.y + 4 * 9
    };
    fb->PrintText(this->big_font, &pmfd_right_weapon_chaff, const_cast<char *>("C:30"), 0, 0, 4, 2, 2);

    Point2D pmfd_right_weapon_flare{
        pmfd_right_weapon.x - 7 + this->cockpit->MONI.MFDS.WEAP.ARTS.GetShape(0)->GetWidth() / 2,
        pmfd_right_weapon.y + 5 * 9
    };
    fb->PrintText(this->big_font, &pmfd_right_weapon_flare, const_cast<char *>("F:30"), 0, 0, 4, 2, 2);
}

void SCCockpit::RenderMFDSRadar(Point2D pmfd_left, float range, int mode, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    switch (mode) {
    case RadarMode::AARD:  
        this->RenderMFDSRadarImplementation(pmfd_left, range, "AIR", true, fb);
        break;
    case RadarMode::AGRD:
        this->RenderMFDSRadarImplementation(pmfd_left, range, "GROUND", false, fb);
        break;
    case RadarMode::ASST:
        break;
    case RadarMode::AFRD:
        break;
    default:
        break;
    }
}

void SCCockpit::RenderMFDSRadarImplementation(Point2D pmfd_left, float range, const char* mode_name, bool air_mode, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    // Calcul des positions et dimensions
    Point2D pmfd_center = {
        pmfd_left.x + this->cockpit->MONI.SHAP.GetWidth() / 2,
        pmfd_left.y + this->cockpit->MONI.SHAP.GetHeight() / 2,
    };
    this->RenderMFDS(pmfd_left, fb);
    
    // Dimensions du radar
    Point2D radar_size = {100, 80};
    Point2D bottom_right = {pmfd_left.x + radar_size.x, pmfd_left.y + radar_size.y};
    
    // Choisir les ressources en fonction du mode
    auto& radar_arts = air_mode ? this->cockpit->MONI.MFDS.AARD.ARTS : this->cockpit->MONI.MFDS.AGRD.ARTS;
    int bg_shape_index = air_mode ? 4 : 1;
    
    // Position du fond du radar
    Point2D radar_bg_pos = pmfd_left;
    
    if (air_mode) {
        radar_bg_pos.x += -5 + this->cockpit->MONI.SHAP.GetWidth() / 2 - 
                        radar_arts.GetShape(bg_shape_index)->GetWidth() / 2;
    } else {
        radar_bg_pos.x += this->cockpit->MONI.SHAP.GetWidth() / 2 - 
                        radar_arts.GetShape(bg_shape_index)->GetWidth();
    }
    
    radar_bg_pos.y += this->cockpit->MONI.SHAP.GetHeight() / 2 - 
                      radar_arts.GetShape(bg_shape_index)->GetHeight() / 2;
    
    // Dessine le fond du radar
    radar_arts.GetShape(bg_shape_index)->SetPosition(&radar_bg_pos);
    fb->DrawShape(radar_arts.GetShape(bg_shape_index));

    // Affiche les infos textuelles
    Point2D pmfd_text = {radar_bg_pos.x + 10, radar_bg_pos.y + 2};
    std::string rzoom_str = "10";
    switch (this->radar_zoom) {
        case 1: rzoom_str = "10"; break;
        case 2: rzoom_str = "20"; break;
        case 3: rzoom_str = "40"; break;
        case 4: rzoom_str = "80"; break;
        default: rzoom_str = "10"; break;
    }
    fb->PrintText(this->big_font, &pmfd_text, 
                 (char *)rzoom_str.c_str(), 0, 0, 2, 2, 2);
    
    std::string mode_text = " " + std::string(mode_name) + " ";
    fb->PrintText(this->big_font, &pmfd_text, const_cast<char *>(mode_text.c_str()), 0, 0, (int) mode_text.length(), 2, 2);
    
    // Affiche le "360" uniquement en mode air
    if (air_mode) {
        fb->PrintText(this->big_font, &pmfd_text, const_cast<char *>("360"), 0, 0, 3, 2, 2);
    }
    
    // Position du joueur comme centre du radar
    Vector2D center = {this->player->position.x, this->player->position.z};
    
    // Préparation pour la rotation
    int heading = (int)this->heading;
    heading = (heading + 360) % 360;  // Normalisation entre 0-359
    float headingRad = heading / 180.0f * (float)M_PI;
    
    // Fonction pour dessiner un contact sur le radar
    auto drawContact = [&](MISN_PART* object, bool isFriendly, bool isDestroyed) {
        // Vérifier si l'entité correspond au mode actuel
        bool valid_entity;
        if (air_mode) {
            valid_entity = (object->entity->entity_type == EntityType::jet);
        } else {
            valid_entity = (object->entity->entity_type == EntityType::ground || 
                           object->entity->entity_type == EntityType::ornt);
        }
        
        if (!valid_entity)
            return;
            
        Vector2D objPos = {object->position.x, object->position.z};
        
        // Rotation selon le heading du joueur
        Vector2D rotatedPos = rotateAroundPoint(objPos, center, headingRad);
        Vector2D relativePos = {rotatedPos.x - center.x, rotatedPos.y - center.y};
        
        // Vérification de la distance
        float distance = sqrtf(relativePos.x * relativePos.x + relativePos.y * relativePos.y);
        if (distance >= range)
            return;
            
        // Échelle et position sur l'écran
        float scale = 60.0f / range;
        Point2D screenPos = {
            pmfd_center.x + (int)(relativePos.x * scale), 
            pmfd_center.y + (int)(relativePos.y * scale)
        };
        
        // Vérifie si le contact est dans les limites du MFD
        if (screenPos.x <= pmfd_left.x || 
            screenPos.x >= pmfd_left.x + this->cockpit->MONI.SHAP.GetWidth() || 
            screenPos.y <= pmfd_left.y || 
            screenPos.y >= pmfd_left.y + this->cockpit->MONI.SHAP.GetHeight())
            return;
            
        // Sélectionne l'icône appropriée
        int iconIndex;
        if (air_mode) {
            if (isFriendly) {
                iconIndex = isDestroyed ? 8 : 9;
            } else {
                iconIndex = isDestroyed ? 5 : 0;
            }
        } else {
            // En mode sol, tous les icônes sont les mêmes (index 3)
            iconIndex = 3;
        }
        
        // Dessine l'icône
        radar_arts.GetShape(iconIndex)->SetPosition(&screenPos);
        fb->DrawShapeWithBox(
            radar_arts.GetShape(iconIndex), 
            pmfd_left.x, bottom_right.x, pmfd_left.y, bottom_right.y
        );
        
        // Si c'est la cible actuelle, dessine le marqueur de cible
        if (object == this->target) {
            Point2D targetPos = {screenPos.x - 2, screenPos.y - 1};
            // Utilise toujours l'icône de cible du mode air
            this->cockpit->MONI.MFDS.AARD.ARTS.GetShape(2)->SetPosition(&targetPos);
            fb->DrawShapeWithBox(
                this->cockpit->MONI.MFDS.AARD.ARTS.GetShape(2), 
                pmfd_left.x, bottom_right.x, pmfd_left.y, bottom_right.y
            );
        }
    };
    
    // Dessine les ennemis
    for (auto actor : this->current_mission->enemies) {
        drawContact(actor->object, false, actor->is_destroyed);
    }
    
    // Dessine les alliés (sauf le joueur)
    for (auto actor : this->current_mission->friendlies) {
        if (actor != this->current_mission->player) {
            drawContact(actor->object, true, actor->is_destroyed);
        }
    }
}

void SCCockpit::RenderRAWS(Point2D pmfd_left = {84,112}, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    Point2D raws_size = {
        this->cockpit->MONI.INST.RAWS.NORM.GetWidth(), 
        this->cockpit->MONI.INST.RAWS.NORM.GetHeight()
    };
    Point2D bottom_right = {
        pmfd_left.x + raws_size.x, pmfd_left.y + raws_size.y
    };
    this->cockpit->MONI.INST.RAWS.NORM.SetPosition(&pmfd_left);
    fb->DrawShape(&this->cockpit->MONI.INST.RAWS.NORM);
    int heading = (int)this->heading;
    heading = (heading + 360) % 360;  // Normalisation entre 0-359
    float headingRad = heading / 180.0f * (float)M_PI;

    for (auto contact: this->current_mission->actors) {
        if (contact->is_active && contact->object->entity->entity_type == EntityType::jet) {
            Vector2D contact_pos = {contact->object->position.x, contact->object->position.z};
            Vector2D center      = {this->player->position.x, this->player->position.z};
            Vector2D roa_dir     = {contact_pos.x - center.x, contact_pos.y - center.y};
            
            float distance = roa_dir.Length();
            roa_dir.Normalize();
            const float max_range = 30000.0f; // 30 km
            if (distance < max_range) {
                float scale = 10.0f;
                scale = (distance / max_range ) * this->cockpit->MONI.INST.RAWS.NORM.GetWidth()/2; // Ajustement de l'échelle
                Point2D p = {
                    (int)(roa_dir.x * scale),
                    (int)(roa_dir.y * scale)
                };
                Point2D rotatedPos = rotateAroundPoint(p, {0,0}, headingRad);
                Point2D raw_pos = {
                    pmfd_left.x + (raws_size.x/2) + rotatedPos.x, 
                    pmfd_left.y + (raws_size.y/2) + rotatedPos.y 
                };
                this->cockpit->MONI.INST.RAWS.SYMB.GetShape(29)->SetPosition(&raw_pos);
                fb->DrawShape(
                    this->cockpit->MONI.INST.RAWS.SYMB.GetShape(29)
                );
                //fb->plot_pixel((int)p.x, (int)p.y, 223);
            }
        }
    }
}

void SCCockpit::RenderMFDSComm(Point2D pmfd_left, int mode, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    this->RenderMFDS(pmfd_left);
    Vector2D center    = {this->player->position.x, this->player->position.y};
    Point2D pmfd_text  = {pmfd_left.x + 20, pmfd_left.y + 20};
    Point2D pfmd_title = {pmfd_text.x + 12, pmfd_text.y};
    fb->PrintText(this->big_font, &pfmd_title, const_cast<char *>("Comm mode:"), 120, 0, 10, 2, 2);
    pfmd_title.y += 10;
    pfmd_title.x = pmfd_left.x + 20;
    fb->PrintText(this->big_font, &pfmd_title, const_cast<char *>("Select frequency"), 0, 0, 16, 2, 2);
    pmfd_text.y += 20;
    if (mode == 0) {
        int cpt = 1;
        for (auto ai : this->current_mission->friendlies) {
            Vector2D ai_position = {ai->object->position.x, ai->object->position.z};
            Vector2D roa_dir     = {ai_position.x - center.x, ai_position.y - center.y};

            float distance = sqrtf((float)(roa_dir.x * roa_dir.x) + (float)(roa_dir.y * roa_dir.y));
            if (ai->actor_name != "PLAYER" && ai->is_active) {
                std::string name_str = std::to_string(cpt) + ". " + ai->actor_name;
                Point2D pfmd_entry   = {pmfd_text.x, pmfd_text.y};
                fb->PrintText(
                    this->big_font, &pfmd_entry, (char *)name_str.c_str(), 120, 0, (uint32_t)name_str.length(), 2, 2
                );
                pmfd_text.y += 10;
                cpt++;
            }
        }
        if (cpt == 1) {
            pfmd_title.y += 25;
            pfmd_title.x = pmfd_left.x + 32;
            fb->PrintText(this->big_font, &pfmd_title, const_cast<char *>("NO RECIVER"), 120, 0, 10, 2, 2);
        }
    } else if (mode > 0) {
        int cpt = 1;
        for (auto ai : this->current_mission->friendlies) {
            if (cpt == mode) {
                int cpt_message = 1;
                for (auto asks : ai->profile->radi.opts) {
                    std::string toask    = std::string("") + asks;
                    std::string request  = this->current_mission->player->profile->radi.asks.at(toask);
                    Point2D pfmd_entry   = {pmfd_text.x, pmfd_text.y};
                    std::string asks_str = std::to_string(cpt_message) + ". " + request;
                    fb->PrintText(
                        this->big_font, &pfmd_entry, (char *)asks_str.c_str(), 124, 0, (uint32_t)asks_str.length(), 2, 2
                    );
                    pmfd_text.y += 6;
                }
                break;
            }
            cpt++;
        }
    }
}

/**
 * Render the cockpit in its current state.
 *
 * If the face number is non-negative, renders the cockpit in its 2D
 * representation using the VGA graphics mode. Otherwise, renders the
 * cockpit in its 3D representation using the OpenGL graphics mode.
 *
 * @param face The face number of the cockpit to render, or -1 to render
 * in 3D.
 */
void SCCockpit::Render(int face) {
    FrameBuffer *fb {nullptr};
    if (face >= 0) {
        VGA.SwithBuffers();
        VGA.Activate();
        fb=VGA.GetFrameBuffer();
        fb->Clear();
        VGA.SetPalette(&this->palette);
        
        if (face == 0) {
            this->RenderHUD();
            
            fb->blit(this->hud_framebuffer->framebuffer, 114,6, this->hud_framebuffer->width,this->hud_framebuffer->height);
            if (this->target != this->player) {
                this->RenderTargetWithCam();
            }
            if (this->player_plane->weaps_load[this->player_plane->selected_weapon] != nullptr) {
                switch (this->player_plane->weaps_load[this->player_plane->selected_weapon]->objct->wdat->weapon_id) {
                case ID_20MM:
                    this->RenderTargetingReticle();
                    this->radar_mode = RadarMode::AARD;
                    break;
                case ID_AIM9J:
                case ID_AIM9M:
                case ID_AIM120:
                    this->radar_mode = RadarMode::AARD;
                    break;
                case ID_MK20:
                case ID_MK82:
                case ID_DURANDAL:
                    this->RenderBombSight();
                    this->radar_mode = RadarMode::AGRD;
                    break;
                case ID_AGM65D:
                case ID_GBU15:
                    this->radar_mode = RadarMode::AGRD;
                    break;
                case ID_LAU3:
                    this->radar_mode = RadarMode::AGRD;
                    break;
                }
            }

            fb->plot_pixel(161, 50, 223);
        }
        fb->DrawShape(this->cockpit->ARTP.GetShape(face));
        if (face == 0) {
            this->RenderRAWS({84, 112}, fb);
            this->RenderAlti({161,166}, fb);
            this->RenderSpeedOmetter({125, 166}, fb);
            Point2D pmfd_right = {0, 200 - this->cockpit->MONI.SHAP.GetHeight()};
            Point2D pmfd_left  = {
                320 - this->cockpit->MONI.SHAP.GetWidth() - 1, 200 - this->cockpit->MONI.SHAP.GetHeight()
            };
            Point2D pmfd;
            bool mfds = false;
            if (this->show_radars) {
                if (!mfds) {
                    pmfd = pmfd_left;
                    mfds = true;
                } else {
                    pmfd = pmfd_right;
                }
                float range = 0.0f;
                switch (this->radar_zoom) {
                case 1:
                    range = 18520.0f;
                    break;
                case 2:
                    range = 18520.0f * 2.0f;
                case 3:
                    range = 18520.0f * 4.0f;
                    break;
                case 4:
                    range = 18520.0f * 8.0f;
                    break;    
                }
                this->RenderMFDSRadar(pmfd, range, this->radar_mode);
            }
            if (this->show_weapons) {
                if (!mfds) {
                    pmfd = pmfd_left;
                    mfds = true;
                } else {
                    pmfd = pmfd_right;
                }
                this->RenderMFDSWeapon(pmfd);
            }
            if (this->show_comm) {
                if (!mfds) {
                    pmfd = pmfd_left;
                    mfds = true;
                } else {
                    pmfd = pmfd_right;
                }
                this->RenderMFDSComm(pmfd, this->comm_target);
            }
        }
        if (this->current_mission->radio_messages.size() > 0) {
            if (this->radio_mission_timer == 0) {
                if (this->current_mission->radio_messages[0]->sound != nullptr) {
                    Mixer.PlaySoundVoc(this->current_mission->radio_messages[0]->sound->data, 
                                       this->current_mission->radio_messages[0]->sound->size);
                }
                this->radio_mission_timer = 400;
            }
            Point2D radio_text = {4, 9};
            RLEShape *background_message = this->cockpit->PLAQ.shapes.GetShape(2);
            //background_message->SetPosition(&radio_text);
            fb->DrawShape(background_message);
            RSFont *fnt = this->cockpit->PLAQ.fonts[1].font;
            std::string radio_message = this->current_mission->radio_messages[0]->message;
            std::transform(radio_message.begin(), radio_message.end(), radio_message.begin(), ::toupper);
            fb->PrintText(
                fnt, &radio_text, (char *)radio_message.c_str(), 0, 0,
                (uint32_t)radio_message.size(), 2, 2
            );
            if (this->radio_mission_timer > 0) {
                this->radio_mission_timer--;
            }
            if (this->radio_mission_timer <= 0) {
                this->current_mission->radio_messages.erase(this->current_mission->radio_messages.begin());
                this->radio_mission_timer = 0;
            }
        }
        if (this->mouse_control) {
            Mouse.Draw();
        }
        VGA.VSync();
        VGA.SwithBuffers();
    }
}
void SCCockpit::Update() {
    this->yaw_speed         = this->yaw - (this->player_plane->azimuthf / 10.0f);
    this->pitch_speed       = this->pitch - (this->player_plane->elevationf / 10.0f);
    this->roll_speed        = this->roll - (this->player_plane->twist / 10.0f);
    this->pitch             = this->player_plane->elevationf / 10.0f;
    this->roll              = this->player_plane->twist / 10.0f;
    this->yaw               = this->player_plane->azimuthf / 10.0f;
    this->speed             = (float)this->player_plane->airspeed;
    this->mach              = this->player_plane->mach;
    this->g_limit           = this->player_plane->Lmax;
    this->g_load            = this->player_plane->g_load;
    this->throttle          = this->player_plane->GetThrottle();
    this->altitude          = this->player_plane->y;
    this->heading           = this->player_plane->azimuthf / 10.0f;
    this->gear              = this->player_plane->GetWheel();
    this->flaps             = this->player_plane->GetFlaps() > 0;
    this->airbrake          = this->player_plane->GetSpoilers() > 0;
    this->target            = this->target;
    this->player            = this->player_plane->object;
    this->weapoint_coords.x = this->current_mission->waypoints[*this->nav_point_id]->spot->position.x;
    this->weapoint_coords.y = this->current_mission->waypoints[*this->nav_point_id]->spot->position.z;
    this->ai_planes         = this->ai_planes;
    this->player_prof       = this->player_prof;
    this->player_plane      = this->player_plane;
}

void SCCockpit::RenderHUD() {
    FrameBuffer *hud = this->hud_framebuffer;
    hud->FillWithColor(255);
    this->RenderHudHorizonLinesSmall({46, 44}, hud);
    this->RenderAltitude({72, 24}, hud);
    this->RenderSpeed({0, 24}, hud);
    this->RenderHeading({22, 82}, hud);

    std::string txt;
    char speed_buffer[10];
    Point2D g_load_text = {7, 14};
    snprintf(speed_buffer, sizeof(speed_buffer), "%.2f", this->g_load);
    txt = speed_buffer;
    hud->PrintText(this->font, &g_load_text, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2);
    Point2D g_limit_text = {7, g_load_text.y+5};
    snprintf(speed_buffer, sizeof(speed_buffer), "%.2f", this->g_limit);
    txt = speed_buffer;
    hud->PrintText(this->font, &g_limit_text, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2);



    Point2D speed_text = {7, 24+40};
    snprintf(speed_buffer, sizeof(speed_buffer), "%.3f", this->mach);
    txt = speed_buffer;
    hud->PrintText(this->font, &speed_text, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2);
    Point2D throttle_text = {7, speed_text.y + 5};
    txt                   = std::to_string(this->throttle);
    hud->PrintText(this->font, &throttle_text, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2);
    


    Point2D inrange_text = {7, throttle_text.y + 5};
    Point2D weapons_text = {7+16, throttle_text.y + 10};
    Point2D weapons_count_text = {7, weapons_text.y};
    std::map<int, std::string> weapon_names = {
        {ID_20MM, "GUNS"},
        {ID_AIM9J, "AIM-9J"},
        {ID_AIM9M, "AIM-9M"},
        {ID_AIM120, "AIM-120"},
        {ID_MK20, "MK-20"},
        {ID_MK82, "MK-82"},
        {ID_DURANDAL, "DURANDAL"},
        {ID_AGM65D, "AGM-65D"},
        {ID_GBU15, "GBU-15"},
        {ID_LAU3, "LAU-3"}
    };
    int weapons_count = 0;
    bool in_range = false;
    if (this->player_plane->weaps_load[this->player_plane->selected_weapon] != nullptr) {
        int weapon_id = this->player_plane->weaps_load[this->player_plane->selected_weapon]->objct->wdat->weapon_id;
        weapons_count = this->player_plane->weaps_load[this->player_plane->selected_weapon]->nb_weap;
        if (this->target != nullptr) {
            uint32_t weap_range = this->player_plane->weaps_load[this->player_plane->selected_weapon]->objct->wdat->effective_range;
            Vector3D dist_to_target = this->target->position - this->player_plane->object->position;
            float distance = dist_to_target.Length();
            if (distance <= weap_range) {
                in_range = true;
            }
        }
        if (weapon_names.find(weapon_id) != weapon_names.end()) {
            txt = weapon_names[weapon_id];
        } else {
            txt = "UNKNOWN";
        }
    } else {
        txt = "NO WEAP";
    }
    hud->PrintText(this->font, &weapons_text, const_cast<char *>(txt.c_str()), 0, 0, (uint32_t)txt.length(), 2, 2);
    Point2D center = {hud->width / 2, hud->height / 2};
    center.x -= this->hud->small_hud->LADD->VECT->SHAP->GetWidth() / 2;
    center.y -= this->hud->small_hud->LADD->VECT->SHAP->GetHeight() / 2;
    center.x += (int) (this->player_plane->ax*1000.0f);
    center.y -= (int) (this->player_plane->ay*100.0f);
    this->hud->small_hud->LADD->VECT->SHAP->SetPosition(&center);
    hud->DrawShapeWithBox(this->hud->small_hud->LADD->VECT->SHAP,0, hud_framebuffer->width,0, hud_framebuffer->height);
    if (weapons_count > 0) {
        txt = std::to_string(weapons_count);
    } else {
        txt = "0";
    }
    hud->PrintText(this->font, &weapons_count_text, (char *)txt.c_str(), 0, 0, (uint32_t)txt.length(), 2, 2);
    if (in_range) {
        hud->PrintText(this->font, &inrange_text, const_cast<char *>("IN RANGE"), 0, 0, 8, 2, 2);
    }
    Point2D gear = {75, 9};
    Point2D flaps = {75, gear.y + 5};
    Point2D break_text = {75, flaps.y + 5};
    if (this->gear) {
        hud->PrintText(this->font, &gear, const_cast<char *>("GEARS"), 0, 0, 5, 2, 2);
    }

    if (this->flaps) {
        hud->PrintText(this->font, &flaps, const_cast<char *>("FLAPS"), 0, 0, 5, 2, 2);
    }
    
    if (this->airbrake) {
        hud->PrintText(this->font, &break_text, const_cast<char *>("BREAKS"), 0, 0, 6, 2, 2);
    }
    Point2D pcenter = {hud->width / 2, hud->height / 2};
    hud->plot_pixel(pcenter.x, pcenter.y, 223);
    //hud->rect_slow(0,0, hud->width - 1, hud->height - 1, 1);

}
void SCCockpit::RenderAlti(Point2D pmfd_left = {177,179}, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    RLEShape *shape = this->cockpit->MONI.INST.ALTI.ARTS.GetShape(0);
    Point2D raws_size = {
        shape->GetWidth(),
        shape->GetHeight()
    };
    Point2D bottom_right = {
        pmfd_left.x + raws_size.x, pmfd_left.y + raws_size.y
    };
    shape->SetPosition(&pmfd_left);
    fb->DrawShape(shape);
    // Calculate altitude in feet
    float altiInFeet = this->altitude * 3.28084f;
        
    // Calculate angles for each needle
    // 1000s needle (full circle = 10,000 feet)
    float thousandsAngle = (altiInFeet / 10000.0f) * 2.0f * (float) M_PI;
    // 100s needle (full circle = 1,000 feet)
    float hundredsAngle = (fmodf(altiInFeet, 1000.0f) / 1000.0f) * 2.0f * (float)  M_PI;

    // Calculate center position of the altimeter
    Point2D center = {
        pmfd_left.x + raws_size.x / 2,
        pmfd_left.y + raws_size.y / 2
    };

    // Calculate needle lengths
    int thousandsLength = raws_size.x / 2 - 5;
    int hundredsLength = raws_size.x / 2 - 6;

    // Calculate needle endpoints
    Point2D thousandsEnd = {
        center.x + (int)(thousandsLength * sinf(thousandsAngle)),
        center.y - (int)(thousandsLength * cosf(thousandsAngle))
    };

    Point2D hundredsEnd = {
        center.x + (int)(hundredsLength * sinf(hundredsAngle)),
        center.y - (int)(hundredsLength * cosf(hundredsAngle))
    };

    // Draw needles
    fb->line(center.x, center.y, thousandsEnd.x, thousandsEnd.y, 223);  // Thousands needle
    fb->line(center.x, center.y, hundredsEnd.x, hundredsEnd.y, 90);     // Hundreds needle (different color)
}
void SCCockpit::RenderSpeedOmetter(Point2D pmfd_left = {125,166}, FrameBuffer *fb = VGA.GetFrameBuffer()) {
    RLEShape *shape = this->cockpit->MONI.INST.AIRS.ARTS.GetShape(0);
    Point2D raws_size = {
        shape->GetWidth(),
        shape->GetHeight()
    };
    Point2D bottom_right = {
        pmfd_left.x + raws_size.x, pmfd_left.y + raws_size.y
    };
    shape->SetPosition(&pmfd_left);
    fb->DrawShape(shape);
    // Calculate speed in knots
    float speedInKnots = this->speed;
    // Calculate angle for the needle
    float speedAngle = (speedInKnots / 1500.0f) * 2.0f * (float)  M_PI; // Assuming max speed is 600 knots
    // Calculate center position of the speedometer
    Point2D center = {
        pmfd_left.x + raws_size.x / 2,
        pmfd_left.y + raws_size.y / 2
    };
    // Calculate needle length
    int needleLength = raws_size.x / 2 - 5; // Adjusted for the size of the speedometer
    // Calculate needle endpoint
    Point2D needleEnd = {
        center.x + (int)(needleLength * sinf(speedAngle)),
        center.y - (int)(needleLength * cosf(speedAngle))
    };
    // Draw the needle
    fb->line(center.x, center.y, needleEnd.x, needleEnd.y, 223); // Draw the speed needle

}