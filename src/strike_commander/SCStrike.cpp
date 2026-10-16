//
//  SCStrike.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"
#include <cctype>
#include <tuple>
#include <optional>
#include <algorithm>
#include <cmath>
#include "SCStrike.h"
#include "../realspace/block_def.h"
#include "../engine/gametimer.h"
#define SC_WORLD 1100
#define RENDER_DISTANCE 40000.0f
#define AUTOPILOTE_TIMEOUT 1000
#define AUTOPILOTE_SPEED 4

static Matrix invertRigidBodyMatrix(const Matrix& m) {
    Matrix out;
    out.Identity();
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            out.v[c][r] = m.v[r][c];
        }
    }
    const float tx = m.v[3][0];
    const float ty = m.v[3][1];
    const float tz = m.v[3][2];
    out.v[3][0] = -(out.v[0][0] * tx + out.v[1][0] * ty + out.v[2][0] * tz);
    out.v[3][1] = -(out.v[0][1] * tx + out.v[1][1] * ty + out.v[2][1] * tz);
    out.v[3][2] = -(out.v[0][2] * tx + out.v[1][2] * ty + out.v[2][2] * tz);
    return out;
}

static void drawTargetSquareOverlay(int32_t viewportW,
                                int32_t viewportH,
                                const Matrix& projection,
                                const Matrix& view,
                                const Vector3D& targetWorldPos) {
    Vector3DHomogeneous v = {targetWorldPos.x, targetWorldPos.y, targetWorldPos.z, 1.0f};
    Matrix viewM = view;
    Matrix projM = projection;
    Vector3DHomogeneous viewPos = viewM.multiplyMatrixVector(v);
    Vector3DHomogeneous clipPos = projM.multiplyMatrixVector(viewPos);

    if (clipPos.w <= 0.0f) {
        return;
    }

    const float ndcX = clipPos.x / clipPos.w;
    const float ndcY = clipPos.y / clipPos.w;
    const float ndcZ = clipPos.z / clipPos.w;

    if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f || ndcZ < -1.0f || ndcZ > 1.0f) {
        return;
    }

    const int px = (int)((ndcX * 0.5f + 0.5f) * (float)viewportW);
    const int py = (int)((0.5f - ndcY * 0.5f) * (float)viewportH);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (double)viewportW, (double)viewportH, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glColor3f(0.0f, 1.0f, 0.0f);
    const float s = 8.0f;
    glBegin(GL_LINE_LOOP);
    glVertex2f((float)px - s, (float)py - s);
    glVertex2f((float)px + s, (float)py - s);
    glVertex2f((float)px + s, (float)py + s);
    glVertex2f((float)px - s, (float)py + s);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

}
void SCStrike::cleanupVirtualCockpitTextures() {
    static std::vector<Texture *> s_PrevFrameGLTex;
    static std::vector<Texture *> s_CurrentFrameGLTex;
    
    for (Texture* tex : s_PrevFrameGLTex) {
        delete tex;
    }
    s_PrevFrameGLTex.clear();
    s_PrevFrameGLTex.shrink_to_fit();
    
    for (Texture* tex : s_CurrentFrameGLTex) {
        delete tex;
    }
    s_CurrentFrameGLTex.clear();
    s_CurrentFrameGLTex.shrink_to_fit();
}
void SCStrike::renderVirtualCockpit() {
    if (this->cockpit->is_f16_cockpit) {
        renderVirtualF16Cockpit();
    } else {
        renderVirtualF22Cockpit();
    }
}
void SCStrike::renderVirtualF16Cockpit() {
    if (this->cockpit->cockpit == nullptr) {
        return;
    }
    static std::vector<Texture *> s_PrevFrameGLTex;
    static std::vector<Texture *> s_CurrentFrameGLTex;

    if (!s_PrevFrameGLTex.empty()) {
        for (Texture* tex : s_PrevFrameGLTex) {
            delete tex;
        }
        s_PrevFrameGLTex.clear();
        s_PrevFrameGLTex.shrink_to_fit();
    }
    s_PrevFrameGLTex.swap(s_CurrentFrameGLTex); 
    const Vector3D camPos = {
        this->camera->getPosition().x,
        this->camera->getPosition().y,
        this->camera->getPosition().z
    };
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(camPos.x, camPos.y, camPos.z);

    Vector3D cockpit_pos = {0.0f, 0.0f, 0.0f};

    

    Vector3D cockpit_rot;
    cockpit_rot = {(this->player_plane->azimuthf+900)/10.0f,
                    this->player_plane->elevationf/10.0f,
                    -this->player_plane->twist/10.0f};

    Vector3D cockpit_ajustement = { 0.0f,-(float) this->eye_y,0.0f};
    if (this->camera->isUsingCustomMatrices()) {
        const float cockpitScale = 1.0f; // ajuste: 0.7 .. 0.95
        //cockpit_ajustement = { -0.225f,-(float) this->eye_y+2.35f,0.0f};
        cockpit_ajustement = { -0.55f,-(float) this->eye_y,0.0f};
        Renderer.drawModel(this->cockpit->cockpit->REAL.OBJS, Renderer.lodLevel, cockpit_pos, cockpit_rot, cockpit_ajustement, cockpitScale);
    } else {
        Renderer.drawModel(this->cockpit->cockpit->REAL.OBJS, Renderer.lodLevel, cockpit_pos, cockpit_rot, cockpit_ajustement);
    }

    
    this->cockpit->cannonAngularOffset = gunsight_hud_offset;
    CockpitFace prev_face = this->cockpit->face;
    this->cockpit->face = CockpitFace::CP_BIG;
    this->cockpit->RenderHUD();
    this->cockpit->face = prev_face;
    if (this->cockpit->hud != nullptr) {
        Texture *hud_texture = new Texture();
        hud_texture->animated = true;
        RSImage *hud_image = new RSImage();
        hud_image->palette = &this->cockpit->palette;
        hud_image->data = this->cockpit->hud_framebuffer->framebuffer;
        hud_image->width = this->cockpit->hud_framebuffer->width;
        hud_image->height = this->cockpit->hud_framebuffer->height;
        hud_texture->set(hud_image);
        hud_texture->updateContent(hud_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {5.8f, 2.0f, -1.22f}, 
                {5.8f, 2.0f, 1.35f},
                {6.0f, -0.8f, 1.35f},
                {6.0f, -0.8f, -1.22f}
            },
            hud_texture
        );
        hud_image->data = nullptr;
        delete hud_image;
        s_CurrentFrameGLTex.push_back(hud_texture);
    }
    if (this->cockpit->cockpit->MONI.SHAP.data != nullptr) {        
        Texture *mfd_right_texture = new Texture();
        mfd_right_texture->animated = true;
        RSImage *mfd_right_image = new RSImage();
        mfd_right_image->palette = &this->cockpit->palette;
        if (this->cockpit->show_cam) {
            cockpit->RenderMFDSCamera({0,0}, cockpit->mfd_right_framebuffer);
        } else if (this->cockpit->show_damage) {
            cockpit->RenderMFDSDamage({0,0}, cockpit->mfd_right_framebuffer);
        }else {
            cockpit->RenderMFDSWeapon({0,0}, cockpit->mfd_right_framebuffer);
        }
        
        mfd_right_image->data = this->cockpit->mfd_right_framebuffer->framebuffer;
        mfd_right_image->width = this->cockpit->mfd_right_framebuffer->width;
        mfd_right_image->height = this->cockpit->mfd_right_framebuffer->height;
        mfd_right_texture->set(mfd_right_image);
        mfd_right_texture->updateContent(mfd_right_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.2f, -2.5f, -3.0f}, 
                {6.2f, -2.5f, -1.15f},
                {6.1f, -4.4f, -1.15f},
                {6.1f, -4.4f, -3.0f}
            },
            mfd_right_texture
        );
        mfd_right_image->data = nullptr;
        delete mfd_right_image;
        s_CurrentFrameGLTex.push_back(mfd_right_texture);
        if (this->cockpit->show_comm) {
            cockpit->mfd_left_framebuffer->fillWithColor(0);
            cockpit->RenderMFDSComm({0,0}, cockpit->comm_target, cockpit->mfd_left_framebuffer);
        } else {
            cockpit->RenderMFDSRadar({0,0}, cockpit->radar_zoom*20000.0f, this->cockpit->radar_mode, cockpit->mfd_left_framebuffer);
        }
        Texture *mfd_left_texture = new Texture();
        mfd_left_texture->animated = true;
        RSImage *mfd_left_image = new RSImage();
        mfd_left_image->palette = &this->cockpit->palette;
        mfd_left_image->data = this->cockpit->mfd_left_framebuffer->framebuffer;
        mfd_left_image->width = this->cockpit->mfd_left_framebuffer->width;
        mfd_left_image->height = this->cockpit->mfd_left_framebuffer->height;
        mfd_left_texture->set(mfd_left_image);
        mfd_left_texture->updateContent(mfd_left_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.2f, -2.5f, 1.15f}, 
                {6.2f, -2.5f, 3.0f},
                {6.1f, -4.4f, 3.0f},
                {6.1f, -4.4f, 1.15f}
            },
            mfd_left_texture
        );
        mfd_left_image->data = nullptr;
        delete mfd_left_image;
        s_CurrentFrameGLTex.push_back(mfd_left_texture);
    }
    if (this->cockpit->cockpit->MONI.INST.RAWS.NORM.data != nullptr) {
        cockpit->raws_framebuffer->fillWithColor(0);
        cockpit->RenderRAWSBig({0,0}, cockpit->raws_framebuffer);
        Texture *raws_texture = new Texture();
        raws_texture->animated = true;
        RSImage *raws_image = new RSImage();
        raws_image->palette = &this->cockpit->palette;
        raws_image->data = this->cockpit->raws_framebuffer->framebuffer;
        raws_image->width = this->cockpit->raws_framebuffer->width;
        raws_image->height = this->cockpit->raws_framebuffer->height;
        raws_texture->set(raws_image);
        raws_texture->updateContent(raws_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.2f, -1.38f, -2.10f}, 
                {6.2f, -1.38f, -1.15f},
                {6.1f, -2.45f, -1.15f},
                {6.1f, -2.45f, -2.10f}
            },
            raws_texture
        );
        raws_image->data = nullptr;
        delete raws_image;
        s_CurrentFrameGLTex.push_back(raws_texture);
    }
    if (cockpit->target_framebuffer != nullptr && !camera->isUsingCustomMatrices()) {
    
        cockpit->target_framebuffer->fillWithColor(255);
        cockpit->RenderTargetWithCam({0,0}, cockpit->target_framebuffer);
        Texture *target_texture = new Texture();
        target_texture->animated = true;
        RSImage *target_image = new RSImage();
        target_image->palette = &this->cockpit->palette;
        target_image->data = this->cockpit->target_framebuffer->framebuffer;
        target_image->width = this->cockpit->target_framebuffer->width;
        target_image->height = this->cockpit->target_framebuffer->height;
        target_texture->set(target_image);
        target_texture->updateContent(target_image);
        // Calculate a position in front of the camera
        // Create a fullscreen quad in front of the camera
        // Adjust for the camera Y-axis offset
        Vector3D quadPos = camera->getPosition() + camera->getForward() * 8.0f;
        
        // Set up vectors for billboard creation
        Vector3D forward = camera->getForward();
        Vector3D right = camera->getRight();
        Vector3D up = camera->getUp();

        
        
        // Scale factors for quad size
        float width =4.92f;
        float height = 3.22f;
        // Compensate for the 0.45f Y-axis projection offset
        // We need to shift the entire quad upwards by adding an offset in the up direction
        float projectionYOffset = -1.5f;
        Vector3D offsetCompensation = up * projectionYOffset;
        
        // Apply the offset to the quad center position
        Vector3D adjustedQuadPos = quadPos + offsetCompensation;
        
        // Define the quad's corners in world space with the adjusted center position
        Vector3D topLeft = adjustedQuadPos + (up * height) - (right * width);
        Vector3D topRight = adjustedQuadPos + (up * height) + (right * width);
        Vector3D bottomRight = adjustedQuadPos - (up * height) + (right * width);
        Vector3D bottomLeft = adjustedQuadPos - (up * height) - (right * width);
        
        // Draw the quad with the target texture
        Renderer.drawTexturedQuad(
            {0, 0, 0}, // No position offset needed as we're using world coordinates
            {0, 0, 0}, // No rotation needed as we're manually setting vertices
            {
                topLeft,
                topRight,
                bottomRight,
                bottomLeft
            },
            target_texture
        );

        target_image->data = nullptr;
        delete target_image;
        s_CurrentFrameGLTex.push_back(target_texture);
    }
    if (this->cockpit->alti_framebuffer != nullptr && this->cockpit->speed_framebuffer != nullptr) {
        cockpit->alti_framebuffer->fillWithColor(0);
        cockpit->RenderAlti({0,0}, cockpit->alti_framebuffer);
        Texture *alti_texture = new Texture();
        alti_texture->animated = true;
        RSImage *alti_image = new RSImage();
        alti_image->palette = &this->cockpit->palette;
        alti_image->data = this->cockpit->alti_framebuffer->framebuffer;
        alti_image->width = this->cockpit->alti_framebuffer->width;
        alti_image->height = this->cockpit->alti_framebuffer->height;
        alti_texture->set(alti_image);
        alti_texture->updateContent(alti_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.1f, -3.15f, 0.020f}, 
                {6.1f, -3.15f, 0.90f},
                {5.7f, -3.92f, 0.90f},
                {5.7f, -3.92f, 0.020f}
            },
            alti_texture
        );

        alti_image->data = nullptr;    
        delete alti_image;
        s_CurrentFrameGLTex.push_back(alti_texture);


        cockpit->speed_framebuffer->fillWithColor(0);
        cockpit->RenderSpeedOmetter({0,0}, cockpit->speed_framebuffer);
        Texture *speed_texture = new Texture();
        speed_texture->animated = true;
        RSImage *speed_image = new RSImage();
        speed_image->palette = &this->cockpit->palette;
        speed_image->data = this->cockpit->speed_framebuffer->framebuffer;
        speed_image->width = this->cockpit->speed_framebuffer->width;
        speed_image->height = this->cockpit->speed_framebuffer->height;
        speed_texture->set(speed_image);
        speed_texture->updateContent(speed_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.1f, -3.15f, -0.90f}, 
                {6.1f, -3.15f, -0.020f},
                {5.7f, -3.92f, -0.020f},
                {5.7f, -3.92f, -0.90f}
            },
            speed_texture
        );

        speed_image->data = nullptr;        
        delete speed_image;
        s_CurrentFrameGLTex.push_back(speed_texture);
        cockpit->comm_framebuffer->fillWithColor(0);

        if (cockpit->RenderCommMessages({0,13}, cockpit->comm_framebuffer)) {
            Texture *comm_texture = new Texture();
            comm_texture->animated = true;
            RSImage *comm_image = new RSImage();
            comm_image->palette = &this->cockpit->palette;
            comm_image->data = this->cockpit->comm_framebuffer->framebuffer;
            comm_image->width = this->cockpit->comm_framebuffer->width;
            comm_image->height = this->cockpit->comm_framebuffer->height;
            comm_texture->set(comm_image);
            comm_texture->updateContent(comm_image);
            Renderer.drawTexturedQuad(
                cockpit_pos,
                cockpit_rot,
                {
                    {6.1f, -0.95f, -1.90f}, 
                    {6.1f, -0.95f, 2.020f},
                    {6.1f, -1.10f, 2.020f},
                    {6.1f, -1.10f, -1.90f}
                },
                comm_texture
            );
            comm_image->data = nullptr;        
            delete comm_image;
            s_CurrentFrameGLTex.push_back(comm_texture);   
        }
        if (this->mouse_control) {
            this->virtual_mouse_cockpit_buffer->fillWithColor(0);
            this->virtual_mouse_cockpit_buffer->line(160,0,160,200, 223);
            this->virtual_mouse_cockpit_buffer->line(0,100,320,100, 223);
            this->virtual_mouse_cockpit_buffer->rect_slow(1,1,319,199, 223);
            Point2D cursorPos = Mouse.position;
            cursorPos.x -= 4;
            cursorPos.y -= 4;
            Mouse.appearances[SCMouse::CURSOR]->SetPosition(&cursorPos);
            this->virtual_mouse_cockpit_buffer->drawShape(Mouse.appearances[SCMouse::CURSOR]);
            Texture *mouse_texture = new Texture();
            mouse_texture->animated = true;
            RSImage *mouse_image = new RSImage();
            mouse_image->palette = &this->cockpit->palette;
            mouse_image->data = this->virtual_mouse_cockpit_buffer->framebuffer;
            mouse_image->width = this->virtual_mouse_cockpit_buffer->width;
            mouse_image->height = this->virtual_mouse_cockpit_buffer->height;
            mouse_texture->set(mouse_image);
            mouse_texture->updateContent(mouse_image);
            Renderer.drawTexturedQuad(
                cockpit_pos,
                cockpit_rot,
                {
                    {6.05f, -1.10f, -1.0f}, 
                    {6.05f, -1.10f, 1.0f},
                    {6.05f, -2.10f, 1.0f},
                    {6.05f, -2.10f, -1.0f}
                },
                mouse_texture
            );
            mouse_image->data = nullptr;        
            delete mouse_image;
            s_CurrentFrameGLTex.push_back(mouse_texture);
        }
        
    }
    glPopMatrix();
}
void SCStrike::renderVirtualF22Cockpit() {
    if (this->cockpit->cockpit == nullptr) {
        return;
    }
    static std::vector<Texture *> s_PrevFrameGLTex;
    static std::vector<Texture *> s_CurrentFrameGLTex;

    if (!s_PrevFrameGLTex.empty()) {
        for (Texture* tex : s_PrevFrameGLTex) {
            delete tex;
        }
        s_PrevFrameGLTex.clear();
        s_PrevFrameGLTex.shrink_to_fit();
    }
    s_PrevFrameGLTex.swap(s_CurrentFrameGLTex); 
    const Vector3D camPos = {
        this->camera->getPosition().x,
        this->camera->getPosition().y,
        this->camera->getPosition().z
    };
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(camPos.x, camPos.y, camPos.z);

    Vector3D cockpit_pos = {0.0f, 0.0f, 0.0f};

    

    Vector3D cockpit_rot;
    cockpit_rot = {(this->player_plane->azimuthf+900)/10.0f,
                    this->player_plane->elevationf/10.0f,
                    -this->player_plane->twist/10.0f};

    Vector3D cockpit_ajustement = { 0.0f,-(float) this->eye_y,0.0f};
    if (this->camera->isUsingCustomMatrices()) {
        const float cockpitScale = 1.0f; // ajuste: 0.7 .. 0.95
        //cockpit_ajustement = { -0.225f,-(float) this->eye_y+2.35f,0.0f};
        cockpit_ajustement = { -0.55f,-(float) this->eye_y,0.0f};
        Renderer.drawModel(this->cockpit->cockpit->REAL.OBJS, Renderer.lodLevel, cockpit_pos, cockpit_rot, cockpit_ajustement, cockpitScale);
    } else {
        Renderer.drawModel(this->cockpit->cockpit->REAL.OBJS, Renderer.lodLevel, cockpit_pos, cockpit_rot, cockpit_ajustement);
    }

    
    this->cockpit->cannonAngularOffset = gunsight_hud_offset;
    CockpitFace prev_face = this->cockpit->face;
    this->cockpit->face = CockpitFace::CP_BIG;
    this->cockpit->RenderHUD();
    this->cockpit->face = prev_face;
    if (this->cockpit->hud != nullptr) {
        Texture *hud_texture = new Texture();
        hud_texture->animated = true;
        RSImage *hud_image = new RSImage();
        hud_image->palette = &this->cockpit->palette;
        hud_image->data = this->cockpit->hud_framebuffer->framebuffer;
        hud_image->width = this->cockpit->hud_framebuffer->width;
        hud_image->height = this->cockpit->hud_framebuffer->height;
        hud_texture->set(hud_image);
        hud_texture->updateContent(hud_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {5.8f, 2.0f, -1.22f}, 
                {5.8f, 2.0f, 1.35f},
                {6.0f, -0.8f, 1.35f},
                {6.0f, -0.8f, -1.22f}
            },
            hud_texture
        );
        hud_image->data = nullptr;
        delete hud_image;
        s_CurrentFrameGLTex.push_back(hud_texture);
    }
    if (this->cockpit->cockpit->MONI.SHAP.data != nullptr) {        
        Texture *mfd_right_texture = new Texture();
        mfd_right_texture->animated = true;
        RSImage *mfd_right_image = new RSImage();
        mfd_right_image->palette = &this->cockpit->palette;
        if (this->cockpit->show_cam) {
            cockpit->RenderMFDSCamera({0,0}, cockpit->mfd_right_framebuffer);
        } else if (this->cockpit->show_damage) {
            cockpit->RenderMFDSDamage({0,0}, cockpit->mfd_right_framebuffer);
        }else {
            cockpit->RenderMFDSWeapon({0,0}, cockpit->mfd_right_framebuffer);
        }
        
        mfd_right_image->data = this->cockpit->mfd_right_framebuffer->framebuffer;
        mfd_right_image->width = this->cockpit->mfd_right_framebuffer->width;
        mfd_right_image->height = this->cockpit->mfd_right_framebuffer->height;
        mfd_right_texture->set(mfd_right_image);
        mfd_right_texture->updateContent(mfd_right_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {8.2f, -2.95f, -3.3f}, 
                {8.2f, -2.95f, -1.35f},
                {8.1f, -5.0f, -1.35f},
                {8.1f, -5.0f, -3.3f}
            },
            mfd_right_texture
        );
        mfd_right_image->data = nullptr;
        delete mfd_right_image;
        s_CurrentFrameGLTex.push_back(mfd_right_texture);
        if (this->cockpit->show_comm) {
            cockpit->mfd_left_framebuffer->fillWithColor(0);
            cockpit->RenderMFDSComm({0,0}, cockpit->comm_target, cockpit->mfd_left_framebuffer);
        } else {
            cockpit->RenderMFDSRadar({0,0}, cockpit->radar_zoom*20000.0f, this->cockpit->radar_mode, cockpit->mfd_left_framebuffer);
        }
        Texture *mfd_left_texture = new Texture();
        mfd_left_texture->animated = true;
        RSImage *mfd_left_image = new RSImage();
        mfd_left_image->palette = &this->cockpit->palette;
        mfd_left_image->data = this->cockpit->mfd_left_framebuffer->framebuffer;
        mfd_left_image->width = this->cockpit->mfd_left_framebuffer->width;
        mfd_left_image->height = this->cockpit->mfd_left_framebuffer->height;
        mfd_left_texture->set(mfd_left_image);
        mfd_left_texture->updateContent(mfd_left_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {8.2f, -2.95f, 1.35f}, 
                {8.2f, -2.95f, 3.3f},
                {8.1f, -5.0f, 3.3f},
                {8.1f, -5.0f, 1.35f}
            },
            mfd_left_texture
        );
        mfd_left_image->data = nullptr;
        delete mfd_left_image;
        s_CurrentFrameGLTex.push_back(mfd_left_texture);
    }
    if (this->cockpit->cockpit->MONI.INST.RAWS.NORM.data != nullptr) {
        cockpit->raws_framebuffer->fillWithColor(0);
        cockpit->RenderRAWSBig({0,0}, cockpit->raws_framebuffer);
        Texture *raws_texture = new Texture();
        raws_texture->animated = true;
        RSImage *raws_image = new RSImage();
        raws_image->palette = &this->cockpit->palette;
        raws_image->data = this->cockpit->raws_framebuffer->framebuffer;
        raws_image->width = this->cockpit->raws_framebuffer->width;
        raws_image->height = this->cockpit->raws_framebuffer->height;
        raws_texture->set(raws_image);
        raws_texture->updateContent(raws_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {5.8f, -0.95f, -0.54f}, 
                {5.8f, -0.95f, 0.64f},
                {5.8f, -1.835f, 0.64f},
                {5.8f, -1.835f, -0.54f}
            },
            raws_texture
        );
        raws_image->data = nullptr;
        delete raws_image;
        s_CurrentFrameGLTex.push_back(raws_texture);
    }
    if (cockpit->target_framebuffer != nullptr && !camera->isUsingCustomMatrices()) {
    
        cockpit->target_framebuffer->fillWithColor(255);
        cockpit->RenderTargetWithCam({0,0}, cockpit->target_framebuffer);
        Texture *target_texture = new Texture();
        target_texture->animated = true;
        RSImage *target_image = new RSImage();
        target_image->palette = &this->cockpit->palette;
        target_image->data = this->cockpit->target_framebuffer->framebuffer;
        target_image->width = this->cockpit->target_framebuffer->width;
        target_image->height = this->cockpit->target_framebuffer->height;
        target_texture->set(target_image);
        target_texture->updateContent(target_image);
        // Calculate a position in front of the camera
        // Create a fullscreen quad in front of the camera
        // Adjust for the camera Y-axis offset
        Vector3D quadPos = camera->getPosition() + camera->getForward() * 8.0f;
        
        // Set up vectors for billboard creation
        Vector3D forward = camera->getForward();
        Vector3D right = camera->getRight();
        Vector3D up = camera->getUp();

        
        
        // Scale factors for quad size
        float width =4.92f;
        float height = 3.22f;
        // Compensate for the 0.45f Y-axis projection offset
        // We need to shift the entire quad upwards by adding an offset in the up direction
        float projectionYOffset = -1.5f;
        Vector3D offsetCompensation = up * projectionYOffset;
        
        // Apply the offset to the quad center position
        Vector3D adjustedQuadPos = quadPos + offsetCompensation;
        
        // Define the quad's corners in world space with the adjusted center position
        Vector3D topLeft = adjustedQuadPos + (up * height) - (right * width);
        Vector3D topRight = adjustedQuadPos + (up * height) + (right * width);
        Vector3D bottomRight = adjustedQuadPos - (up * height) + (right * width);
        Vector3D bottomLeft = adjustedQuadPos - (up * height) - (right * width);
        
        // Draw the quad with the target texture
        Renderer.drawTexturedQuad(
            {0, 0, 0}, // No position offset needed as we're using world coordinates
            {0, 0, 0}, // No rotation needed as we're manually setting vertices
            {
                topLeft,
                topRight,
                bottomRight,
                bottomLeft
            },
            target_texture
        );

        target_image->data = nullptr;
        delete target_image;
        s_CurrentFrameGLTex.push_back(target_texture);
    }
        
    if (cockpit->RenderCommMessages({0,13}, cockpit->comm_framebuffer)) {
        cockpit->comm_framebuffer->fillWithColor(0);

        Texture *comm_texture = new Texture();
        comm_texture->animated = true;
        RSImage *comm_image = new RSImage();
        comm_image->palette = &this->cockpit->palette;
        comm_image->data = this->cockpit->comm_framebuffer->framebuffer;
        comm_image->width = this->cockpit->comm_framebuffer->width;
        comm_image->height = this->cockpit->comm_framebuffer->height;
        comm_texture->set(comm_image);
        comm_texture->updateContent(comm_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.1f, -0.95f, -1.90f}, 
                {6.1f, -0.95f, 2.020f},
                {6.1f, -1.10f, 2.020f},
                {6.1f, -1.10f, -1.90f}
            },
            comm_texture
        );
        comm_image->data = nullptr;        
        delete comm_image;
        s_CurrentFrameGLTex.push_back(comm_texture);   
    }
    if (this->mouse_control) {
        this->virtual_mouse_cockpit_buffer->fillWithColor(0);
        this->virtual_mouse_cockpit_buffer->line(160,0,160,200, 223);
        this->virtual_mouse_cockpit_buffer->line(0,100,320,100, 223);
        this->virtual_mouse_cockpit_buffer->rect_slow(1,1,319,199, 223);
        Point2D cursorPos = Mouse.position;
        cursorPos.x -= 4;
        cursorPos.y -= 4;
        Mouse.appearances[SCMouse::CURSOR]->SetPosition(&cursorPos);
        this->virtual_mouse_cockpit_buffer->drawShape(Mouse.appearances[SCMouse::CURSOR]);
        Texture *mouse_texture = new Texture();
        mouse_texture->animated = true;
        RSImage *mouse_image = new RSImage();
        mouse_image->palette = &this->cockpit->palette;
        mouse_image->data = this->virtual_mouse_cockpit_buffer->framebuffer;
        mouse_image->width = this->virtual_mouse_cockpit_buffer->width;
        mouse_image->height = this->virtual_mouse_cockpit_buffer->height;
        mouse_texture->set(mouse_image);
        mouse_texture->updateContent(mouse_image);
        Renderer.drawTexturedQuad(
            cockpit_pos,
            cockpit_rot,
            {
                {6.05f, -1.10f, -1.0f}, 
                {6.05f, -1.10f, 1.0f},
                {6.05f, -2.10f, 1.0f},
                {6.05f, -2.10f, -1.0f}
            },
            mouse_texture
        );
        mouse_image->data = nullptr;        
        delete mouse_image;
        s_CurrentFrameGLTex.push_back(mouse_texture);
    }
    glPopMatrix();
}
/**
 * @brief Constructor
 *
 * Initializes the SCStrike object with a default camera position and
 * other variables.
 */
SCStrike::SCStrike() {
    this->camera_mode = 0;
    this->camera_pos = {-169.0f, 79.0f, -189.0f};
    this->counter = 0;
    if (Game == nullptr) {
        Game = &GameEngine::instance();
    }
    this->m_keyboard = Game->getKeyboard();
    if (this->Screen == nullptr) {
        this->Screen = &RSScreen::instance();
    }
}

SCStrike::~SCStrike() {
    Game->direct_mouse_control = false;
}
void SCStrike::registerSimulatorInputs() {
    
}
void SCStrike::autopilotCompute() {
    Vector2D destination = {this->current_mission->waypoints[this->nav_point_id]->spot->position.x,
                            this->current_mission->waypoints[this->nav_point_id]->spot->position.z};
    this->autopilot_target_azimuth = atan2(this->player_plane->x-destination.x, this->player_plane->z-destination.y);
    this->autopilot_target_azimuth = radToDegree(this->autopilot_target_azimuth);
    this->player_plane->yaw = 0.0f;
    this->player_plane->pitch = 0.0f;
    this->player_plane->roll = 0.0f;
    float dest_y = this->current_mission->waypoints[this->nav_point_id]->spot->position.y;
    Vector2D origine = {this->player_plane->x, this->player_plane->z};
    std::vector<Vector2D> path;
    for (auto area: this->current_mission->mission->mission_data.areas) {
        {
            bool area_active = false;
            for (auto scen: this->current_mission->mission->mission_data.scenes) {
                if (scen->area_id == area->id-1) {
                    area_active = area_active || scen->is_active;
                }
            }
            if (!area_active) {
                continue;
            }
            printf("check collision with Areas: %s\n", area->AreaName);
            // Assume each area is represented as an axis-aligned rectangle in the X-Z plane.
            // Compute rectangle boundaries.
            float halfWidth  = area->AreaWidth / 2.0f;
            float halfHeight = area->AreaHeight / 2.0f;
            float left   = area->position.x - halfWidth;
            float right  = area->position.x + halfWidth;
            float top    = area->position.z - halfHeight;
            float bottom = area->position.z + halfHeight;

            // Lambda to test if a point is inside the rectangle.
            auto pointInRect = [&](const Vector2D &pt) -> bool {
                return (pt.x >= left && pt.x <= right && pt.y >= top && pt.y <= bottom);
            };
            if (pointInRect(origine)) {
                // we know we are in this area already
                printf("We are in area: %s\n", area->AreaName);
                continue;
            }
            // If either endpoint is inside, we have a collision.
            /*if (pointInRect(origine) || pointInRect(destination)) {
                printf("Collision detected with area: %s\n", area->AreaName);
                continue;
            }*/

            auto doLinesIntersect = [&](const Vector2D &p1, const Vector2D &p2,
                                            const Vector2D &p3, const Vector2D &p4) -> std::optional<Vector2D> {
                float r_px = p2.x - p1.x;
                float r_py = p2.y - p1.y;
                float s_px = p4.x - p3.x;
                float s_py = p4.y - p3.y;
                float denominator = r_px * s_py - r_py * s_px;
                if (fabs(denominator) < 1e-6f)
                    return std::nullopt; // lines are parallel

                float t = ((p3.x - p1.x) * s_py - (p3.y - p1.y) * s_px) / denominator;
                float u = ((p3.x - p1.x) * r_py - (p3.y - p1.y) * r_px) / denominator;

                if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
                    // Return the intersection point
                    return Vector2D { p1.x + t * r_px, p1.y + t * r_py };
                }
                return std::nullopt;
            };

            // Define rectangle edges (using two points per edge).
            Vector2D r1 = {left, top};
            Vector2D r2 = {right, top};
            Vector2D r3 = {right, bottom};
            Vector2D r4 = {left, bottom};

            std::optional<Vector2D> i1 = doLinesIntersect(origine, destination, r1, r2);
            std::optional<Vector2D> i2 = doLinesIntersect(origine, destination, r2, r3);
            std::optional<Vector2D> i3 = doLinesIntersect(origine, destination, r3, r4);
            std::optional<Vector2D> i4 = doLinesIntersect(origine, destination, r4, r1);

            // Check if the segment (origin,destination) intersects any rectangle edge.
            if (i1 != std::nullopt ||
                i2 != std::nullopt ||
                i3 != std::nullopt ||
                i4 != std::nullopt)
            {
                printf("Collision detected with area: %s\n", area->AreaName);
                //continue;
            }
            if (i1 != std::nullopt) {
                path.push_back(i1.value());
            }
            if (i2 != std::nullopt) {
                path.push_back(i2.value());
            }
            if (i3 != std::nullopt) {
                path.push_back(i3.value());
            }
            if (i4 != std::nullopt) {
                path.push_back(i4.value());
            }
        }
    }
    if (!path.empty()) {
        Vector2D selectedPoint;
        float minDistSq = (std::numeric_limits<float>::max)();
        for (const auto &pt : path) {
            float dx = pt.x - origine.x;
            float dy = pt.y - origine.y;
            float distSq = dx * dx + dy * dy;
            if (distSq < minDistSq) {
                minDistSq = distSq;
                selectedPoint = pt;
            }
        }
        printf("Selected point: (%.3f, %.3f)\n", selectedPoint.x, selectedPoint.y);
        
        
        this->player_plane->x = selectedPoint.x;
        this->player_plane->z = selectedPoint.y;
        
    } else {
        printf("No intersection points found in path.\n");
        
        this->player_plane->x = destination.x;
        this->player_plane->z = destination.y;
    }
    float dest_min_altitude = this->current_mission->area->getY(this->player_plane->x, this->player_plane->z);
    if (dest_y<dest_min_altitude) {
        this->player_plane->y += dest_min_altitude;
    } else {
        this->player_plane->y = dest_y;    
    }
    
    
    
    this->player_plane->ptw.Identity();
    this->player_plane->ptw.translateM(this->player_plane->x, this->player_plane->y, this->player_plane->z);
    this->player_plane->ptw.rotateM(0, 0, 1, 0);
    this->player_plane->ptw.rotateM(0, 1, 0, 0);
    this->player_plane->ptw.rotateM(0, 0, 0, 1);
    this->player_plane->Simulate();
    this->player_plane->alignVelocityToNose();
    Vector3D formation_pos_offset{80.0f, 0.0f, 40.0f};
    int team_number = 1;
    Vector3D prev={this->player_plane->x, this->player_plane->y, this->player_plane->z};
    for (auto team: this->current_mission->friendlies) {
        if (team->is_active) {
            if (team->plane != nullptr && team->plane != this->player_plane) {
                // ailier en mode cinematique (formation, pilote automatique) : simulateKinematic ecraserait l'attitude
                if (team->pilot != nullptr) {
                    team->pilot->disengageAutopilot();
                    team->pilot->CmdKinematic(false, team->plane->worldVelocity(), team->plane->forward, team->pilot->NosePitch());
                    MessageBus::getInstance().processEvents();
                }
                prev += formation_pos_offset;
                team->taken_off = true;
                team->plane->x = prev.x;
                team->plane->z = prev.z;
                team->plane->y = prev.y;
                team->plane->yaw=0;
                team->plane->pitch=0;
                team->plane->roll=0;
                team->plane->ptw.Identity();
                team->plane->ptw.translateM(team->plane->x, team->plane->y, team->plane->z);
                team->plane->ptw.rotateM(0, 0, 1, 0);
                team->plane->ptw.rotateM(0, 1, 0, 0);
                team->plane->ptw.rotateM(0, 0, 0, 1);
                team->plane->Simulate();
                team->plane->alignVelocityToNose();
                team_number++;
            }
        }
    }
    this->camera_mode = View::AUTO_PILOT;
    this->autopilot_timeout = 400;
}

/**
 * @brief Handle keyboard events
 *
 * This function peeks at the SDL event queue to check for keyboard events.
 * If a key is pressed, it updates the game state based on the key pressed.
 * The supported keys are escape, space, and return.
 *
 * @return None
 */
void SCStrike::checkKeyboard(void) {
    int msx = 0;
    int msy = 0;
    
    float dx = m_keyboard->getActionValue(CreateAction(InputAction::SIM_START, SimActionOfst::MOUSE_X));
    float dy = m_keyboard->getActionValue(CreateAction(InputAction::SIM_START, SimActionOfst::MOUSE_Y));

    msy = dy - (Screen->logical_height / 2);
    msx = dx - (Screen->logical_width / 2);
    //Mouse.setPosition({(int)dx, (int)dy});
    if (this->camera_mode == View::AUTO_PILOT) {
        return;
    }
    if (!this->autopilot) {
        if (this->mouse_control) {
            this->player_plane->control_stick_x = msx;
            this->player_plane->control_stick_y = msy;
        } else {
            this->player_plane->control_stick_x = 0;
            this->player_plane->control_stick_y = 0;
        }
    }
    if (this->camera_mode == View::REAL) {
        this->pilote_lookat.x = ((Screen->width / 360) * msx) / 6;
        this->pilote_lookat.y = ((Screen->height / 360) * msy) / 6;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::END_MISSION))) {
        this->current_mission->mission_ended = true;
        Mixer.stopSound();
        Mixer.stopSound(5);
    }
    if (this->player_plane->crached || this->player_plane->ejected) {
        return;
    }
    bool is_rudder_pressed = false;
    if (m_keyboard->isActionPressed(CreateAction(InputAction::SIM_START, SimActionOfst::RUDDER_LEFT))) {
        this->player_plane->rudder -= 0.1f;
        is_rudder_pressed = true;
    }
    if (m_keyboard->isActionPressed(CreateAction(InputAction::SIM_START, SimActionOfst::RUDDER_RIGHT))) {
        this->player_plane->rudder += 0.1f;
        is_rudder_pressed = true;
    }
    if (!is_rudder_pressed) {
        this->player_plane->rudder = 0;
    } else {
        this->player_plane->rudder = (std::clamp)(this->player_plane->rudder, -10.0f, 10.0f);
    }
    if (m_keyboard->isActionPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_UP))) {
        if (this->player_plane->GetThrottle() == 0) {
            this->player_plane->SetThrottle(21);
            if (this->current_mission->sound.sounds.size() > 0) {
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_START_MIL];
                Mixer.playSoundVoc(engine->data, engine->size, 5, 0);
            }
        }
        if (this->player_plane->GetThrottle() < 100) {
            this->player_plane->SetThrottle(this->player_plane->GetThrottle() + 1);
        }
        if (this->current_mission->sound.sounds.size() > 0) {
            if (!Mixer.isSoundPlaying(5)) {
                if (this->player_plane->GetThrottle() > 0 && this->player_plane->GetThrottle() < 60) {
                    MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                    Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
                } else if (this->player_plane->GetThrottle() >= 60) {
                    MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
                    Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
                }
            }
        }
    }
    
    if (m_keyboard->isActionPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_DOWN))) {
        if (this->player_plane->GetThrottle() > -30) {
            this->player_plane->SetThrottle(this->player_plane->GetThrottle() - 1);
        }
        if (this->player_plane->GetThrottle() == 0) {
            if (!Mixer.isSoundPlaying(5)) {
                if (this->player_plane->GetThrottle() > 0 && this->player_plane->GetThrottle() < 60) {
                    MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                    Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
                } else if (this->current_mission->sound.sounds.size() > 0) {
                    MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL_SHUT_DOWN];
                    Mixer.playSoundVoc(engine->data, engine->size, 5, 0);
                }    
            }
            
        }
    }
    this->cockpit->is_shooting = false;
    if (m_keyboard->isActionPressed(CreateAction(InputAction::SIM_START, SimActionOfst::FIRE_PRIMARY))) {
        if (target != nullptr) {
            // WeaponSystem_FrameUpdate_3F8C0 : le missile ne recoit la cible que si le point d'emport est accroche (+0x0B)
            SCMissionActors *weapon_target = target;
            SCWeaponLoadoutHardPoint *station = this->player_plane->weaps_load[this->player_plane->selected_weapon];
            if (station != nullptr && station->objct->wdat->weapon_aspec != 0 && !this->player_plane->seekerLocks(station->objct, target)) {
                weapon_target = nullptr;
            }
            this->player_plane->Shoot(this->player_plane->selected_weapon, weapon_target, this->current_mission);
            this->cockpit->is_shooting = true;
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::TOGGLE_MOUSE))) {
        this->mouse_control = !this->mouse_control;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::AUTOPILOT))) {
        this->autopilotCompute();
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::LANDING_GEAR))) {
        this->player_plane->SetWheel();
        if (this->current_mission->sound.sounds.size() > 0) {
            if (this->player_plane->GetWheel()) {
                MemSound *gears = this->current_mission->sound.sounds[SoundEffectIds::GEARS_UP];
                Mixer.playSoundVoc(gears->data, gears->size, 4, 0);
            } else {
                MemSound *gears = this->current_mission->sound.sounds[SoundEffectIds::GEARS_DOWN];
                Mixer.playSoundVoc(gears->data, gears->size, 4, 0);
            }
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::TOGGLE_BRAKES))) {
        this->player_plane->SetSpoilers();
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::TOGGLE_FLAPS))) {
        this->player_plane->SetFlaps();
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::TARGET_NEAREST))) {
        this->findTarget();
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::MDFS_RADAR))) {
        this->cockpit->show_radars = !this->cockpit->show_radars;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::MDFS_DAMAGE))) {
        this->cockpit->show_damage = !this->cockpit->show_damage;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::MDFS_WEAPONS))) {
        if (this->cockpit->show_weapons) {
            int nb_hardpoint = this->player_plane->object->entity->hpts.size() / 2 +1;
            int next_weapon = (this->player_plane->selected_weapon + 1) % nb_hardpoint;
            if (this->air_weapons_mode) {
                while (this->player_plane->weaps_load[next_weapon]->objct->wdat->weapon_id != weapon_ids::ID_20MM &&
                       this->player_plane->weaps_load[next_weapon]->objct->wdat->weapon_id != weapon_ids::ID_AIM120 &&
                       this->player_plane->weaps_load[next_weapon]->objct->wdat->weapon_id != weapon_ids::ID_AIM9J &&
                       this->player_plane->weaps_load[next_weapon]->objct->wdat->weapon_id != weapon_ids::ID_AIM9M) {
                    next_weapon = (next_weapon + 1) % nb_hardpoint;
                    if (next_weapon == this->player_plane->selected_weapon) {
                        break;
                    }
                }
            }
            if ((this->player_plane->weaps_load[this->player_plane->selected_weapon] != nullptr) && (this->player_plane->weaps_load[this->player_plane->selected_weapon]->objct->wdat->weapon_id == weapon_ids::ID_20MM) && (!this->air_weapons_mode && this->cockpit->weapon_mode == Hud_weapon_mode::WM_HUD_LCOS)) {
                this->cockpit->weapon_mode = Hud_weapon_mode::WM_HUD_STRAF;
            } else {
                this->player_plane->selected_weapon = next_weapon;
                if (this->player_plane->weaps_load[this->player_plane->selected_weapon] == nullptr) {
                    this->cockpit->weapon_mode = Hud_weapon_mode::WM_HUD_NONE;
                } else {
                    switch (this->player_plane->weaps_load[this->player_plane->selected_weapon]->objct->wdat->weapon_id) {
                        case weapon_ids::ID_20MM:
                            this->cockpit->weapon_mode = Hud_weapon_mode::WM_HUD_LCOS;
                            break;
                        case weapon_ids::ID_AIM120:
                            this->cockpit->weapon_mode = Hud_weapon_mode::WM_HUD_LRM;
                            break;
                        case weapon_ids::ID_AIM9J:
                        case weapon_ids::ID_AIM9M:
                            this->cockpit->weapon_mode = Hud_weapon_mode::WM_HUD_SRM;
                            break;
                        case weapon_ids::ID_AGM65D:
                        case weapon_ids::ID_GBU15:
                            this->cockpit->weapon_mode = Hud_weapon_mode::WM_HUD_IRST;
                            break;
                        case weapon_ids::ID_MK20:
                        case weapon_ids::ID_MK82:
                        case weapon_ids::ID_DURANDAL:
                            this->cockpit->weapon_mode = Hud_weapon_mode::WM_HUD_CCIP;
                            break;
                        case weapon_ids::ID_LAU3:
                            this->cockpit->weapon_mode = Hud_weapon_mode::WM_HUD_STRAF;
                            break;
                        default:
                            this->cockpit->weapon_mode = Hud_weapon_mode::WM_HUD_NONE;
                            break;
                    }
                }
            }
            
            
            this->mfd_timeout = 400;
        } else {
            this->cockpit->show_weapons = !this->cockpit->show_weapons;
            this->mfd_timeout = 400;
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::SHOW_NAVMAP))) {
        SCNavMap *nav_screen = new SCNavMap();
        nav_screen->init();
        nav_screen->SetName((char *)this->current_mission->world->tera.c_str());
        nav_screen->mission = this->current_mission;
        nav_screen->missionObj = this->current_mission->mission = this->current_mission->mission;
        nav_screen->current_nav_point = &this->nav_point_id;
        Game->addActivity(nav_screen);
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::CHAFF))) {
        //this->player_plane->LaunchChaff(this->current_mission);
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::FLARE))) {
        //this->player_plane->LaunchFlares(this->current_mission);
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::RADAR_ZOOM_IN))) {
        this->cockpit->radar_zoom -= 1;
        if (this->cockpit->radar_zoom < 1) {
            this->cockpit->radar_zoom = 1;
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::RADAR_ZOOM_OUT))) {
        this->cockpit->radar_zoom += 1;
        if (this->cockpit->radar_zoom > 4) {
            this->cockpit->radar_zoom = 4;
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::COMM_RADIO))) {
        this->cockpit->show_comm = !this->cockpit->show_comm;
        this->mfd_timeout = 400;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::CHAFF))) {
        this->player_plane->DeployCountermeasure(true);
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::EJECT))) {
        this->cockpit->frame = 0;
        this->camera_mode = View::EJECT_VIEW_P1;
        this->player_plane->ejected = true;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::FLARE))) {
        this->player_plane->DeployCountermeasure(false);
    }
    
    
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::VIEW_TARGET))) {
        //this->camera_mode = View::TARGET;
        CameraViewRequest target_req;
        this->camera_mode = View::CAM_DIRECTOR;
        target_req.camera_type = RSCameraType::RSCAM_TARG;

        SCMissionActors *player = this->current_mission->player;
        SCMissionActors *enemy  = player->target;
        bool already_on_target = this->current_mission->camera_director->activeCameraType() == RSCameraType::RSCAM_TARG;
        bool player_is_subject = this->current_mission->camera_director->activeSubject() == player;

        if (already_on_target && player_is_subject) {
            target_req.subject = enemy;
            target_req.target  = player;
        } else {
            target_req.subject = player;
            target_req.target  = enemy;
        }
        MessageBus::getInstance().publish(std::make_unique<CameraViewRequest>(target_req));
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::VIEW_BEHIND))) {
        /*if (this->camera_mode != View::FOLLOW) {
            this->follow_dynamic = false;
        }
        if (this->camera_mode == View::FOLLOW) {
            this->follow_dynamic = !this->follow_dynamic;
        }
        this->camera_mode = View::FOLLOW;*/
        CameraViewRequest chase;
        this->camera_mode = View::CAM_DIRECTOR;
        chase.camera_type = RSCameraType::RSCAM_CHAS;   // = 3, défini dans RSWorld.h
        chase.subject = this->current_mission->player;
        MessageBus::getInstance().publish(std::make_unique<CameraViewRequest>(chase));
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::VIEW_COCKPIT))) {
        this->mouse_control = false;
        this->zoom_cockpit = false;
        this->camera_mode = View::REAL;
        this->cockpit->is_3d_cockpit = true;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::RADAR_MODE_TOGGLE))) {
        if (this->cockpit->radar_mode == RadarMode::AARD) {
            this->cockpit->radar_mode = RadarMode::AGRD;
        } else if (this->cockpit->radar_mode == RadarMode::AGRD) {
            this->cockpit->radar_mode = RadarMode::AARD;
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::VIEW_WEAPONS))) {
        if (this->camera_mode != View::MISSILE_CAM) {
            this->camera_mode = View::MISSILE_CAM;
        } else {
            this->camera_mode = View::REAL;
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::LOOK_FORWARD))) {
        this->camera_mode = View::FRONT;
        this->cockpit->is_3d_cockpit = false;
        zoom_cockpit = !zoom_cockpit;
        this->pilote_lookat.x = 0;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::LOOK_LEFT))) {
        this->camera_mode = View::LEFT;
        this->pilote_lookat.x = 270;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::LOOK_RIGHT))) {
        this->camera_mode = View::RIGHT;
        this->pilote_lookat.x = 90;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::LOOK_BEHIND))) {
        this->camera_mode = View::REAR;
        this->pilote_lookat.x = 180;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::MDFS_TARGET_CAMERA))) {
        this->cockpit->show_cam = !this->cockpit->show_cam;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::WEAPON_MODE_TOGGLE))) {
        this->air_weapons_mode = !this->air_weapons_mode;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::INFINIT_AMMO_TOGGLE))) {
        this->player_plane->infinite_ammo = !this->player_plane->infinite_ammo;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::SINGLE_TARGET_MODE))) {
        if (this->cockpit->radar_mode != RadarMode::ASST && this->target != nullptr) {
            this->cockpit->radar_mode = RadarMode::ASST;
        } else {
            this->cockpit->radar_mode = RadarMode::AARD;
        }
    }
    if (!this->cockpit->show_comm) {
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_10))) {
            this->player_plane->SetThrottle(10);
            if (this->current_mission->sound.sounds.size() > 0) {
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_20))) {
            this->player_plane->SetThrottle(20);
            if (this->current_mission->sound.sounds.size() > 0) {
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_30))) {
            this->player_plane->SetThrottle(30);
            if (this->current_mission->sound.sounds.size() > 0) {
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_40))) {
            this->player_plane->SetThrottle(40);
            if (this->current_mission->sound.sounds.size() > 0) {
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_50))) {
            this->player_plane->SetThrottle(50);
            if (this->current_mission->sound.sounds.size() > 0) {
                MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_MIL];
                Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
            }
        }
    }
    if (this->cockpit->show_comm) {
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::COMM_RADIO_M1))) {
            if (this->cockpit->comm_target == 0) {
                this->cockpit->SetCommActorTarget(1);
            } else {
                // send message 1 to ai comm_target
                if (this->cockpit->comm_actor != nullptr) {
                    this->cockpit->comm_actor->respondToRadioMessage(1, this->current_mission, this->current_mission->player);
                    this->cockpit->show_comm = false;
                }
                this->cockpit->comm_target = 0;
                this->mfd_timeout = 400;
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::COMM_RADIO_M2))) {
            if (this->cockpit->comm_target == 0) {
                this->cockpit->SetCommActorTarget(2);
            } else {
                if (this->cockpit->comm_actor != nullptr) {
                    this->cockpit->comm_actor->respondToRadioMessage(2, this->current_mission, this->current_mission->player);
                    this->cockpit->show_comm = false;
                }
                this->cockpit->comm_target = 0;
                this->mfd_timeout = 400;
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::COMM_RADIO_M3))) {
            if (this->cockpit->comm_target == 0) {
                this->cockpit->SetCommActorTarget(3);
            } else {
                if (this->cockpit->comm_actor != nullptr) {
                    this->cockpit->comm_actor->respondToRadioMessage(3, this->current_mission, this->current_mission->player);
                    this->cockpit->show_comm = false;
                }
                this->cockpit->comm_target = 0;
                this->mfd_timeout = 400;
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::COMM_RADIO_M4))) {
            if (this->cockpit->comm_target == 0) {
                this->cockpit->SetCommActorTarget(4);
            } else {
                if (this->cockpit->comm_actor != nullptr) {
                    this->cockpit->comm_actor->respondToRadioMessage(4, this->current_mission, this->current_mission->player);
                    this->cockpit->show_comm = false;
                }
                this->cockpit->comm_target = 0;
                this->mfd_timeout = 400;
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::COMM_RADIO_M5))) {
            if (this->cockpit->comm_target == 0) {
                this->cockpit->SetCommActorTarget(5);
            } else {
                if (this->cockpit->comm_actor != nullptr) {
                    this->cockpit->comm_actor->respondToRadioMessage(5, this->current_mission, this->current_mission->player);
                    this->cockpit->show_comm = false;
                }
                this->cockpit->comm_target = 0;
                this->mfd_timeout = 400;
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::COMM_RADIO_M6))) {
            if (this->cockpit->comm_target == 0) {
                this->cockpit->SetCommActorTarget(6);
            } else {
                if (this->cockpit->comm_actor != nullptr) {
                    this->cockpit->comm_actor->respondToRadioMessage(5, this->current_mission, this->current_mission->player);
                    this->cockpit->show_comm = false;
                }
                this->cockpit->comm_target = 0;
                this->mfd_timeout = 400;
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::COMM_RADIO_M7))) {
            if (this->cockpit->comm_target == 0) {
                this->cockpit->SetCommActorTarget(7);
            } else {
                if (this->cockpit->comm_actor != nullptr) {
                    this->cockpit->comm_actor->respondToRadioMessage(5, this->current_mission, this->current_mission->player);
                    this->cockpit->show_comm = false;
                }
                this->cockpit->comm_target = 0;
                this->mfd_timeout = 400;
            }
        }
        if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::COMM_RADIO_M8))) {
            if (this->cockpit->comm_target == 0) {
                this->cockpit->SetCommActorTarget(8);
            } else {
                if (this->cockpit->comm_actor != nullptr) {
                    this->cockpit->comm_actor->respondToRadioMessage(5, this->current_mission, this->current_mission->player);
                    this->cockpit->show_comm = false;
                }
                this->cockpit->comm_target = 0;
                this->mfd_timeout = 400;
            }
        }   
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_60))) {
        this->player_plane->SetThrottle(60);
        if (this->current_mission->sound.sounds.size() > 0) {
            MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
            Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_70))) {
        this->player_plane->SetThrottle(70);
        if (this->current_mission->sound.sounds.size() > 0) {
            MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
            Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_80))) {
        this->player_plane->SetThrottle(80);
        if (this->current_mission->sound.sounds.size() > 0) {
            MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
            Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_90))) {
        this->player_plane->SetThrottle(90);
        if (this->current_mission->sound.sounds.size() > 0) {
            MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
            Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::THROTTLE_100))) {
        this->player_plane->SetThrottle(100);
        if (this->current_mission->sound.sounds.size() > 0) {
            MemSound *engine = this->current_mission->sound.sounds[SoundEffectIds::ENGINE_AFB];
            Mixer.playSoundVoc(engine->data, engine->size, 5, -1);
        }
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::PAUSE))) {
        this->pause_simu = !this->pause_simu;
    }
    if (m_keyboard->isActionPressed(CreateAction(InputAction::SIM_START, SimActionOfst::PITCH_UP))) {
        this->player_plane->control_stick_y = -150;
        this->mouse_control = false;
    }
    if (m_keyboard->isActionPressed(CreateAction(InputAction::SIM_START, SimActionOfst::PITCH_DOWN))) {
        this->player_plane->control_stick_y = 150;
        this->mouse_control = false;
    }
    if (m_keyboard->isActionPressed(CreateAction(InputAction::SIM_START, SimActionOfst::ROLL_LEFT))) {
        this->player_plane->control_stick_x = -200;
        this->mouse_control = false;
    }
    if (m_keyboard->isActionPressed(CreateAction(InputAction::SIM_START, SimActionOfst::ROLL_RIGHT))) {
        this->player_plane->control_stick_x = 200;
        this->mouse_control = false;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::EYES_ON_TARGET))) {
        if (this->camera_mode != View::EYE_ON_TARGET) {
            this->camera_mode = View::EYE_ON_TARGET;
            this->cockpit->is_3d_cockpit = true;
        } else {
            this->camera_mode = View::FRONT;
            this->cockpit->is_3d_cockpit = false;
        }
    }
    
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::SPEC_KEY_1))) {
        this->current_mission->gameflow_registers[0] = 1;
        this->current_mission->gameflow_registers[1] = 2;
        this->current_mission->mission_ended = true;
    }
    if (m_keyboard->isActionJustPressed(CreateAction(InputAction::SIM_START, SimActionOfst::SPEC_KEY_2))) {
        this->current_mission->gameflow_registers[0] = 0;
        this->current_mission->mission_ended = true;
    }
    float lx = m_keyboard->getActionValue(CreateAction(InputAction::SIM_START, SimActionOfst::CONTROLLER_STICK_LEFT_X)); 
    float ly = m_keyboard->getActionValue(CreateAction(InputAction::SIM_START, SimActionOfst::CONTROLLER_STICK_LEFT_Y)); 

    float rx = m_keyboard->getActionValue(CreateAction(InputAction::SIM_START, SimActionOfst::CONTROLLER_STICK_RIGHT_X));
    float ry = m_keyboard->getActionValue(CreateAction(InputAction::SIM_START, SimActionOfst::CONTROLLER_STICK_RIGHT_Y));

    if (fabs(lx) > 0.1f) {
        this->player_plane->control_stick_x = static_cast<int>(lx * (Screen->width/2.5f));
        this->mouse_control = false;
    }
    if (fabs(ly) > 0.1f) {
        this->player_plane->control_stick_y = static_cast<int>(-ly * (Screen->height/2.5f));
        this->mouse_control = false;
    }
    if (fabs(rx) > 0.1f) {
        this->pilote_lookat.x = static_cast<int>((rx * Screen->width) / 6);
        this->camera_mode = View::CONTROLLER_LOOK;
    }
    if (fabs(ry) > 0.1f) {
        this->pilote_lookat.y = static_cast<int>((ry * Screen->height) / 6);
        this->camera_mode = View::CONTROLLER_LOOK;
    }
    if (fabs(rx) < 0.1f && fabs(ry) < 0.1f && this->camera_mode == View::CONTROLLER_LOOK) {
        this->camera_mode = View::FRONT;
        this->pilote_lookat.x = 0;
        this->pilote_lookat.y = 0;
    }
    this->cockpit->mouse_control = this->mouse_control;
}
void SCStrike::findTarget() {
    const float target_range = 30000.0f;
    const float target_range_sq = target_range * target_range;

    // 1. Collecter tous les ennemis valides dans la portée
    std::vector<int> candidates;
    for (int i = 0; i < (int)this->current_mission->enemies.size(); i++) {
        auto enemy = this->current_mission->enemies[i];
        if (enemy->team_id == this->current_mission->player->team_id) {
            continue;
        }
        if (!enemy->is_active || enemy->is_destroyed) {
            continue;
        }
        float dx = enemy->object->position.x - this->player_plane->x;
        float dy = enemy->object->position.y - this->player_plane->y;
        float dz = enemy->object->position.z - this->player_plane->z;
        float distSq = dx * dx + dy * dy + dz * dz;
        if (distSq <= target_range_sq) {
            candidates.push_back(i);
        }
    }

    if (candidates.empty()) {
        // Aucune cible disponible
        this->current_target = -1;
        this->target = nullptr;
        this->current_mission->player->target = nullptr;
        this->cockpit->target = nullptr;
        return;
    }

    // 2. Trier les candidats par distance croissante
    std::sort(candidates.begin(), candidates.end(), [&](int a, int b) {
        auto ea = this->current_mission->enemies[a];
        auto eb = this->current_mission->enemies[b];
        float dxa = ea->object->position.x - this->player_plane->x;
        float dya = ea->object->position.y - this->player_plane->y;
        float dza = ea->object->position.z - this->player_plane->z;
        float dxb = eb->object->position.x - this->player_plane->x;
        float dyb = eb->object->position.y - this->player_plane->y;
        float dzb = eb->object->position.z - this->player_plane->z;
        return (dxa*dxa + dya*dya + dza*dza) < (dxb*dxb + dyb*dyb + dzb*dzb);
    });

    // 3. Trouver la position de la cible actuelle dans la liste
    int nextIndex = 0; // Par défaut: premier candidat (le plus proche)
    for (int i = 0; i < (int)candidates.size(); i++) {
        if (candidates[i] == this->current_target) {
            // Cycler vers le suivant
            nextIndex = (i + 1) % (int)candidates.size();
            break;
        }
    }

    // 4. Appliquer la nouvelle cible
    int selectedEnemyIndex = candidates[nextIndex];
    this->current_target = selectedEnemyIndex;
    this->target = this->current_mission->enemies[selectedEnemyIndex];
    this->current_mission->player->target = this->target;
    this->cockpit->target = this->target->object;
}
/**
 * SCStrike::Init
 *
 * Initialize the game state with default values and a starting mission.
 *
 * This function initializes the game state by setting the mouse control
 * flag to false, setting the mission to "TEMPLATE.IFF", initializing the
 * cockpit object, and setting the pilot's lookat position to the origin.
 */
void SCStrike::init(void) {
    this->mouse_control = false;
    Game->direct_mouse_control = true;
    this->pilote_lookat = {0, 0};
    this->registerSimulatorInputs();
    Config &cfg = Config::getInstance();
    this->world_lod = cfg.getInt("Game", "world_detail", 0);
    this->virtual_mouse_cockpit_buffer = new FrameBuffer(320, 200);
    GameState.f16_replacements=0;
}

RSEntity * SCStrike::loadWeapon(std::string name) {
    std::string tmpname = Assets.object_root_path + name + ".IFF";
    RSEntity *objct = new RSEntity();
    TreEntry *entry = Assets.GetEntryByName(tmpname);
    if (entry != nullptr) {
        objct->InitFromRAM(entry->data, entry->size, tmpname);
        return objct;
    }
    return nullptr;
}

/**
 * SetMission
 *
 * Sets the current mission to the one specified in the missionName argument.
 *
 * This function initializes the game state by setting the mission to the one
 * specified in the missionName argument, clearing the list of AI planes,
 * initializing the cockpit object, and setting the pilot's lookat position to
 * the origin. It also loads the necessary data from the mission file, such as
 * the player's coordinates and the objects in the mission.
 *
 * @param[in] missionName The name of the mission file to load.
 */
void SCStrike::setMission(char const *missionName) {
    if (Mixer.isSoundPlaying(5)) {
        Mixer.stopSound(5);
    }
    Mixer.stopMusic();
    
    if (this->current_mission != nullptr) {
        this->current_mission->cleanup();
        delete this->current_mission;
        this->current_mission = nullptr;
    } 
    this->miss_file_name = missionName;
    this->current_mission = new SCMission(missionName, &object_cache);
    ai_planes.clear();
    ai_planes.shrink_to_fit();
    
    MISN_PART *playerCoord = this->current_mission->player->object;
    this->area = this->current_mission->area;
    new_position.x = playerCoord->position.x;
    new_position.z = playerCoord->position.z;
    new_position.y = playerCoord->position.y;

    camera = Renderer.getCamera();
    camera->SetPosition(&new_position);
    this->current_target = 0;
    this->target = this->current_mission->enemies[this->current_target];
    this->nav_point_id = 0;
    this->player_plane = this->current_mission->player->plane;
    this->player_plane->yaw = (360 - playerCoord->azymuth) * 10.0f;
    this->player_plane->object = playerCoord;
    float ground = this->area->getY(new_position.x, new_position.z);
    if (fabs(ground - new_position.y) > 10.0f) {
        this->player_plane->SetThrottle(100);
        this->player_plane->SetWheel();
        this->player_plane->vz = -20;
        this->player_plane->Simulate();
    }
    if (GameState.weapon_load_out.size()>1) {
        this->player_plane->object->entity->weaps.clear();
        std::unordered_map<weapon_type_shp_id, std::string> weapon_names = {
            {AIM9J, "SWINDERJ"},
            {AIM9M, "SWINDERM"},
            {AIM120, "AMRAAM"},
            {AGM65D, "AGM-65D"},
            {DURANDAL, "DURANDAL"},
            {MK20, "MK20"},
            {MK82, "MK82"},
            {GBU15, "GBU-15G"},
            {LAU3, "POD"}
        };
        RSEntity::WEAPS *weap = new RSEntity::WEAPS();
        weap->name = "20MM";
        weap->nb_weap = 1000;
        weap->objct = loadWeapon(weap->name);
        this->player_plane->object->entity->weaps.push_back(weap);
        std::unordered_map<weapon_type_shp_id, bool> loaded;
        for (int i=1; i<5; i++) {
            if (loaded.find(weapon_type_shp_id(GameState.weapon_load_out[i])) != loaded.end()) {
                continue;
            }
            RSEntity::WEAPS *weap = new RSEntity::WEAPS();
            weap->name = weapon_names[weapon_type_shp_id(GameState.weapon_load_out[i])];
            weap->nb_weap = GameState.weapon_load_out[GameState.weapon_load_out[i]];
            weap->objct = loadWeapon(weap->name);
            this->player_plane->object->entity->weaps.push_back(weap);
            loaded[weapon_type_shp_id(GameState.weapon_load_out[i])] = true;
        }
    }
    
    this->player_plane->InitLoadout();
    this->player_prof = this->current_mission->player->profile;
    this->cockpit = new SCCockpit();
    this->cockpit->player_plane = this->player_plane;
    this->cockpit->init();
    this->cockpit->current_mission = this->current_mission;
    this->cockpit->nav_point_id = &this->nav_point_id;

    // Déclencheur : plan d'ouverture STARTCAM. camera_mode suit le directeur : on
    // le pose direct sur CAM_DIRECTOR et on synchronise last_director_view.
    SCCameraSequence::s_debug = false;   // trace stdout pour le debug caméra
    this->camera_mode = View::CAM_DIRECTOR;
    this->last_director_view = View::CAM_DIRECTOR;
    CameraViewRequest startcam;
    startcam.view = View::CAM_DIRECTOR;
    startcam.sequence_name = "STARTCAM";
    startcam.subject = this->current_mission->player;
    MessageBus::getInstance().publish(std::make_unique<CameraViewRequest>(startcam));
}
void SCStrike::setCameraFront() {
    Vector3D pos = {this->new_position.x, this->new_position.y, this->new_position.z};
    camera->SetPosition(&pos);
    camera->resetRotate();
    camera->rotate(
        -tenthOfDegreeToRad(this->player_plane->elevationf),
        -tenthOfDegreeToRad(this->player_plane->azimuthf),
        -tenthOfDegreeToRad(this->player_plane->twist)
    );
}
void SCStrike::setCameraFollow(SCPlane *plane) {
    const float distanceBehind = -60.0f;
    float r_azim = tenthOfDegreeToRad(plane->azimuthf);
    float r_elev = tenthOfDegreeToRad(plane->elevationf-100.0f);
    float r_twist = tenthOfDegreeToRad(plane->twist);
    Vector3D camPos;
    camPos.x = plane->x - distanceBehind * cos(r_elev) * sin(r_azim);
    camPos.y = plane->y + distanceBehind * sin(r_elev);
    camPos.z = plane->z - distanceBehind * cos(r_elev) * cos(r_azim);
    camera->SetPosition(&camPos);

    Vector3D camLookAt = {plane->x, plane->y, plane->z};
    float cosT = cos(r_twist), sinT = sin(r_twist);
    float cosE = cos(r_elev), sinE = sin(r_elev);
    float cosA = cos(r_azim), sinA = sin(r_azim);

    // Compute the up vector as the second column of the composite rotation matrix
    // R = Ry(azim) * Rx(elev) * Rz(twist)
    Vector3D up;
    up.x = -cosA * sinT + sinA * sinE * cosT;
    up.y = cosE * cosT;
    up.z = sinA * sinT + cosA * sinE * cosT;
    if (follow_dynamic) {
        camera->lookAt(&camLookAt, &up);
    } else {
        camera->lookAt(&camLookAt);
    }
}
void SCStrike::setCameraRLR() {
    camera->SetPosition(&this->new_position);
    camera->resetRotate();
    camera->rotate(0.0f, this->pilote_lookat.x * ((float)M_PI / 180.0f), 0.0f);
    camera->rotate(
        -tenthOfDegreeToRad(this->player_plane->elevationf),
        -tenthOfDegreeToRad(this->player_plane->azimuthf),
        -tenthOfDegreeToRad(this->player_plane->twist)
    );
}
void SCStrike::setCameraRLRUpDown() {
    camera->SetPosition(&this->new_position);
    camera->resetRotate();
    camera->rotate(-this->pilote_lookat.y * ((float)M_PI / 180.0f), this->pilote_lookat.x * ((float)M_PI / 180.0f), 0.0f);
    camera->rotate(
        -tenthOfDegreeToRad(this->player_plane->elevationf),
        -tenthOfDegreeToRad(this->player_plane->azimuthf),
        -tenthOfDegreeToRad(this->player_plane->twist)
    );
}
void SCStrike::setCameraLookat(Vector3D obj_pos, Vector3D offset) {
    Vector3D pos = {
        this->player_plane->x + offset.x,
        this->player_plane->y + offset.y,
        this->player_plane->z + offset.z
    };
    Vector3D dir = {
        obj_pos.x - pos.x,
        obj_pos.y - pos.y,
        obj_pos.z - pos.z
    };
    Vector3D lookAt;
    Vector3D camPos;

    float len = dir.Length();
    if (len > 20000.0f) {
        Vector3D forward = this->player_plane->forward;
        forward.Normalize();
        lookAt = {
            pos.x + forward.x * 10.0f,
            pos.y + forward.y * 10.0f,
            pos.z + forward.z * 10.0f
        };
        camPos = pos;
    } else {
        dir.Normalize();

        // Calcul de la taille réelle de la cible via sa BoundingBox
        float targetSize = 30.0f; // valeur par défaut
        if (this->target != nullptr && this->target->object != nullptr && this->target->object->entity != nullptr) {
            BoudingBox *bb = this->target->object->entity->GetBoudingBpx();
            if (bb != nullptr) {
                float sizeX = bb->max.x - bb->min.x;
                float sizeY = bb->max.y - bb->min.y;
                float sizeZ = bb->max.z - bb->min.z;
                // On prend la diagonale de la bounding box comme taille de référence
                targetSize = sqrt(sizeX*sizeX + sizeY*sizeY + sizeZ*sizeZ);
            }
        }

        // On veut que la cible occupe toujours TARGET_ANGULAR_SIZE degrés à l'écran
        // d = targetSize / (2 * tan(angle/2))
        const float TARGET_ANGULAR_SIZE = 15.0f; // degrés
        float camDist = targetSize / (2.0f * tan(TARGET_ANGULAR_SIZE * ((float)M_PI / 180.0f) / 2.0f));

        camDist = (std::max)(camDist, 10.0f); // pas trop près
        camDist = (std::min)(camDist, len);   // pas au-delà de la cible

        camPos = {
            obj_pos.x - dir.x * camDist,
            obj_pos.y - dir.y * camDist,
            obj_pos.z - dir.z * camDist
        };
        lookAt = {
            obj_pos.x,
            obj_pos.y + 10.0f,
            obj_pos.z
        };
    }
    Vector3D up;

    float r_twist = tenthOfDegreeToRad(this->player_plane->twist);
    float r_elev  = tenthOfDegreeToRad(this->player_plane->elevationf);
    float r_azim  = tenthOfDegreeToRad(this->player_plane->azimuthf);
    float cosT = cos(r_twist), sinT = sin(r_twist);
    float cosE = cos(r_elev), sinE = sin(r_elev);
    float cosA = cos(r_azim), sinA = sin(r_azim);
    up.x = -cosA * sinT + sinA * sinE * cosT;
    up.y = cosE * cosT;
    up.z = sinA * sinT + cosA * sinE * cosT;
    camera->SetPosition(&camPos);
    camera->lookAt(&lookAt ,&up);
}
void SCStrike::setCameraLookat(Vector3D obj_pos) {
    Vector3D offset = {0, 0, 0};
    this->setCameraLookat(obj_pos, offset);
}
/**
 * @brief Executes a single frame of the game simulation.
 *
 * This function performs the main loop operations for each frame of the game
 * simulation. It handles keyboard input check, updates the states of the player
 * plane and AI planes, adjusts the camera view based on the current mode, and
 * renders the game world and cockpit.
 *
 * Key operations include:
 * - Checking and processing keyboard inputs.
 * - Simulating the player and AI planes' movements and autopilot systems.
 * - Updating positions and orientations of planes and mission objects.
 * - Configuring the camera based on the selected view mode.
 * - Rendering the world, mission objects, and cockpit interface.
 * - Managing the display of cockpit elements like communication and weapons.
 */
void SCStrike::runFrame(void) {
    Mixer.setVolume(5,5);
    this->checkKeyboard();
    View director_view = this->current_mission->camera_director->currentView();
    if (director_view != this->last_director_view) {
        this->camera_mode = director_view;
        this->last_director_view = director_view;
    }
    Renderer.setLight(&this->light);
    if (!this->pause_simu && this->camera_mode!=View::AUTO_PILOT) {
        this->mfd_timeout--;
        this->player_plane->Simulate();
        this->current_mission->update();
        if (this->current_mission->mission_ended) {
            GameState.missions_flags.clear();
            GameState.missions_flags.shrink_to_fit();
            // @TODO Rework this part when we will be dealing with the mission funds bonus
            for (auto flags: this->current_mission->gameflow_registers) {
                GameState.missions_flags.push_back(flags.second);
            }
            //GameState.cash = GameState.proj_cash + this->current_mission->gameflow_registers[1]*1000;
            
            GameState.mission_flyed_success[GameState.mission_flyed] = this->current_mission->gameflow_registers[0]>0;
            //GameState.kill_board[PilotsId::PLAYER][KillBoardType::AIR_KILL] += this->current_mission->player->plane_down;
            //GameState.kill_board[PilotsId::PLAYER][KillBoardType::GROUND_KILL] += this->current_mission->player->ground_down;
            GameState.air_kills += this->current_mission->player->plane_down;
            GameState.ground_kills += this->current_mission->player->ground_down;
            GameState.kill_board[PilotsId::PLAYER][KillBoardType::AIR_KILL] =GameState.air_kills;
            GameState.kill_board[PilotsId::PLAYER][KillBoardType::GROUND_KILL] = GameState.ground_kills;
            if (this->player_plane->crached) {
                GameState.player_dead = true;
            }
            if (this->player_plane->ejected) {
                GameState.player_ejected = true;
                GameState.f16_replacements += 500000;
                GameState.lost_aircraft += 1;
            }
            GameState.cash = GameState.proj_cash - GameState.over_head;
            GameState.proj_cash = GameState.cash + this->current_mission->gameflow_registers[1]*1000 - GameState.weapons_costs - GameState.f16_replacements  ;
            GameState.weapons_costs = 0;
            for (auto team: this->current_mission->friendlies) {
                if (team->is_active) {
                    if (team->plane != nullptr && team->actor_name != "PLAYER") {
                        GameState.kill_board[pilot_profile[team->actor_name]][KillBoardType::AIR_KILL] += team->plane_down;
                        GameState.kill_board[pilot_profile[team->actor_name]][KillBoardType::GROUND_KILL] += team->ground_down;
                    }
                }
            }
            Renderer.clear();
            Screen->refresh();
            Renderer.clear();
            cleanupVirtualCockpitTextures();
            Game->stopTopActivity();
            return;
        }
    }
    if (this->camera_mode == View::CAM_DIRECTOR) {
        // Vue pilotée par le directeur de caméra (CHASE/TARGET/ROTA/COMP) :
        // le FOV vient du fichier (RSCameraDef::fov), pas du zoom cockpit.
        Renderer.camera.fovy = this->current_mission->camera_director->fov();
        Renderer.camera.update();
    } else if (this->zoom_cockpit) {
        Renderer.camera.fovy = 30.0f;
        Renderer.camera.update();
    } else {
        Renderer.camera.fovy = 45.0f;
        Renderer.camera.update();
    }
    this->player_plane->getPosition(&new_position);
    if (this->player_plane->object != nullptr) {
        this->player_plane->object->position.x = new_position.x;
        this->player_plane->object->position.z = new_position.z;
        this->player_plane->object->position.y = new_position.y;
    }
    this->cockpit->Update();
    if (this->mfd_timeout <= 0) {
        if (this->cockpit->show_comm) {
            this->cockpit->show_comm = false;
        }
        if (this->cockpit->show_weapons) {
            this->cockpit->show_weapons = false;
        }
    }

    Vector3D target_position = {0,0,0};
    if (this->target != nullptr) {
        if (this->target->object != nullptr) {
            target_position = {
                this->target->object->position.x, 
                this->target->object->position.y, 
                this->target->object->position.z
            };
        }
    }
    this->setCameraLookat(target_position);
    Renderer.initRenderToTexture();
    Renderer.verticalOffset = 0.0f;
    Renderer.initRenderCameraView();
    Renderer.renderWorldToTexture(area);
    
    if (this->target != nullptr) {
        if (this->target->plane != nullptr) {
            this->target->plane->Render();    
        } else {
            Renderer.drawModel(this->target->object->entity, target_position, {0.0f, 0.0f, 0.0f});
        }
    }
    
    Renderer.getRenderToTexture();
    Renderer.verticalOffset = -0.45f;
    Renderer.initRenderCameraView();
    if (this->player_plane->crached && !this->player_plane->ejected && this->camera_mode != View::CRASH_VIEW_P2 && this->camera_mode != View::CRASH_VIEW_P3 && this->camera_mode != View::CRASH_VIEW_P1) {
        this->camera_mode = View::CRASH_VIEW_P1;
        this->cockpit->frame = 0;
    }
    switch (this->camera_mode) {
    case View::CAM_DIRECTOR: {
        Vector3D scripted_pos = this->current_mission->camera_director->position();
        Vector3D scripted_aim = this->current_mission->camera_director->aimPoint();
        Vector3D scripted_up = this->current_mission->camera_director->up();
        this->camera->SetPosition(&scripted_pos);
        this->camera->lookAt(&scripted_aim, &scripted_up);
    } break;
    case View::AUTO_PILOT: {
        if (this->autopilot_timeout > -AUTOPILOTE_TIMEOUT) {
            this->autopilot_timeout -= AUTOPILOTE_SPEED;
            Vector3D pos = {this->new_position.x +5, this->new_position.y + 5, 
                this->new_position.z - (autopilot_timeout)};
            camera->SetPosition(&pos);
            camera->lookAt(&this->new_position);
        } else {
            this->player_plane->ptw.Identity();
            this->player_plane->ptw.translateM(this->player_plane->x, this->player_plane->y, this->player_plane->z);
            this->player_plane->ptw.rotateM(degreeToRad(this->autopilot_target_azimuth), 0, 1, 0);
            this->player_plane->yaw = this->autopilot_target_azimuth * 10.0f;
            this->player_plane->Simulate();
            this->player_plane->alignVelocityToNose();
            this->camera_mode = View::FRONT;
            for (auto team: this->current_mission->friendlies) {
                if (team->is_active) {
                    if (team->plane != nullptr && team->plane != this->player_plane) {                       
                        team->plane->yaw=this->autopilot_target_azimuth * 10.0f;
                        team->plane->Simulate();
                        team->plane->alignVelocityToNose();
                    }
                }
            }
        }
    } break;
    case View::EJECT_VIEW_P1:
        this->pilote_lookat.y = -45;
        this->pilote_lookat.x = 0;
        this->setCameraRLRUpDown();
    break;
    case View::FRONT: {
        this->setCameraFront();
         // Apply a vertical offset to the projection matrix
    } break;
    case View::FOLLOW: {
        this->setCameraFollow(this->player_plane);
    } break;
    case View::RIGHT:
    case View::LEFT:
    case View::REAR:
        this->setCameraRLR();
        break;
    case View::CRASH_VIEW_P1:
        this->pilote_lookat.x = 180;
        this->setCameraRLR();
    break;
    case View::CRASH_VIEW_P2:
        this->pilote_lookat.x = 0;
        this->setCameraRLR();
    break;
    case View::CRASH_VIEW_P3:
        {   
            static float crash_cam_distance = 20.0f;
            static float crash_cam_distance_max = 100.0f;
            static float crash_cam_travel = 0.1f;
            if (crash_cam_distance > crash_cam_distance_max) {
                crash_cam_distance = crash_cam_distance_max;
            }
            if (crash_cam_travel > crash_cam_distance_max) {
                crash_cam_travel = crash_cam_distance_max;
            }
            crash_cam_distance += 0.1f;
            crash_cam_travel += 0.1f;
            this->setCameraLookat(this->player_plane->position, {crash_cam_travel, 20, crash_cam_distance});
        }
        
    break;
    case View::EJECT_VIEW_P2:
        {
            if (this->ejected_object == nullptr) {
                this->ejected_object = new EjectionSeatSimulatedObject();
                this->ejected_object->x = this->player_plane->position.x;
                this->ejected_object->y = this->player_plane->position.y;
                this->ejected_object->z = this->player_plane->position.z;
                this->ejected_object->mission = this->current_mission;
                this->simulated_objects.push_back(this->ejected_object);
            }
            Vector3D pos = {
                this->ejected_object->x,
                this->ejected_object->y,
                this->ejected_object->z
            };
            if (this->ejected_object->vy < 0.0f) {
                SCSimulatedObject *pilot = this->ejected_object;
                this->ejected_object = new EjectedPilotSimulatedObject();
                this->ejected_object->x = pilot->x;
                this->ejected_object->y = pilot->y;
                this->ejected_object->z = pilot->z;
                this->ejected_object->mission = this->current_mission;
                this->simulated_objects.push_back(this->ejected_object);
                this->camera_mode = View::EJECT_VIEW_P3;
            }
            Vector3D cam_eject_pos = {
                this->ejected_object->x,
                this->ejected_object->y + 15.0f,
                this->ejected_object->z - 20.0f
            };
            this->camera->SetPosition(&cam_eject_pos);
            this->camera->lookAt(&pos);
        }
    break;
    case EJECT_VIEW_P3:
        {
            static float eject_cam_y_offset = 15.0f;
            static float eject_cam_z_offset = -20.0f;
            Vector3D pos = {
                this->ejected_object->x,
                this->ejected_object->y,
                this->ejected_object->z
            };
            Vector3D cam_eject_pos = {
                this->ejected_object->x,
                this->ejected_object->y + eject_cam_y_offset,
                this->ejected_object->z - eject_cam_z_offset
            };
            eject_cam_y_offset -= 0.1f;
            eject_cam_z_offset -= 0.1f;
            if (eject_cam_y_offset < -25.0f) {
                eject_cam_y_offset = -25.0f;
            }
            if (eject_cam_z_offset < -160.0f) {
                eject_cam_z_offset = -160.0f;
            }
            this->camera->SetPosition(&cam_eject_pos);
            this->camera->lookAt(&pos);
        }
    break;
    case View::TARGET: {
        if (this->target == nullptr) {
            this->camera_mode = View::FRONT;
            break;
        }
        if (this->target->object == nullptr) {
            this->camera_mode = View::FRONT;
            break;
        }
        setCameraLookat(this->target->object->position);
    }
    break;
    case View::OBJECT: {
        setCameraLookat(this->current_mission->actors[this->current_object_to_view]->object->position);
    }
    break;
    case View::MISSILE_CAM: {
        if (this->player_plane->weaps_object.size() == 0) {
            this->camera_mode = View::FRONT;
            break;
        }
        SCSimulatedObject *missile = this->player_plane->weaps_object.back();
        if (missile == nullptr) {
            this->camera_mode = View::FRONT;
            break;
        }
        Vector3D missil_pos{0,0,0};
        missile->GetPosition(&missil_pos);
        Vector3D mpos = {
            missil_pos.x+this->camera_pos.x,
            missil_pos.y+this->camera_pos.y,
            missil_pos.z+this->camera_pos.z
        };
        const float distanceBehind = -60.0f;
        float r_azim = degreeToRad(missile->azimuthf);
        float r_elev = degreeToRad(missile->elevationf);
        float r_twist = 0.0f;
        Vector3D camPos;
        camPos.x = missil_pos.x - distanceBehind * cos(r_elev) * sin(r_azim);
        camPos.y = missil_pos.y + distanceBehind * sin(r_elev);
        camPos.z = missil_pos.z - distanceBehind * cos(r_elev) * cos(r_azim);
        camera->SetPosition(&camPos);
        camera->lookAt(&missil_pos);
    }
    break;
    case View::EYE_ON_TARGET: {
        if (this->target == nullptr) {
            this->camera_mode = View::FRONT;
            break;
        }
        Vector3D pos = {this->new_position.x, this->new_position.y + this->eye_y, this->new_position.z};
        

        Vector3D camPos;
        camPos.x = this->new_position.x;
        camPos.y = this->new_position.y;
        camPos.z = this->new_position.z;
        
        // Compute the up vector as the second column of the composite rotation matrix
        // R = Ry(azim) * Rx(elev) * Rz(twist)
        Vector3D up;

        float r_twist = tenthOfDegreeToRad(this->player_plane->twist);
        float r_elev  = tenthOfDegreeToRad(this->player_plane->elevationf);
        float r_azim  = tenthOfDegreeToRad(this->player_plane->azimuthf);
        float cosT = cos(r_twist), sinT = sin(r_twist);
        float cosE = cos(r_elev), sinE = sin(r_elev);
        float cosA = cos(r_azim), sinA = sin(r_azim);
        up.x = -cosA * sinT + sinA * sinE * cosT;
        up.y = cosE * cosT;
        up.z = sinA * sinT + cosA * sinE * cosT;
        
        Vector3D targetPos = {target->object->position.x, target->object->position.y,target->object->position.z };
        camera->SetPosition(&camPos);
        camera->lookAt(&targetPos, &up);
    } break;
    case View::REAL:
    case View::CONTROLLER_LOOK:
    default: {
        Vector3D pos = {this->new_position.x, this->new_position.y + this->eye_y, this->new_position.z};
        camera->SetPosition(&pos);
        camera->resetRotate();

        camera->rotate(-this->pilote_lookat.y * ((float)M_PI / 180.0f), 0.0f, 0.0f);
        camera->rotate(0.0f, this->pilote_lookat.x * ((float)M_PI / 180.0f), 0.0f);

        camera->rotate(
            -tenthOfDegreeToRad(this->player_plane->elevationf),
            -tenthOfDegreeToRad(this->player_plane->azimuthf),
            -tenthOfDegreeToRad(this->player_plane->twist)
        );
    } break;
    }

    auto renderScene = [&](int32_t viewportW, int32_t viewportH, float projVerticalOffset, bool forceVirtualCockpit) {
        Renderer.bindCameraProjectionAndViewViewport(viewportW, viewportH, projVerticalOffset);
        Renderer.renderWorldSolid(area, this->world_lod, 400);

        if (this->show_bbox) {
            for (auto rrarea: this->current_mission->mission->mission_data.areas) {
                Renderer.renderLineCube(rrarea->position, rrarea->AreaWidth);
            }
        }

        for (auto actor: this->current_mission->actors) {
            if (actor->is_hidden) {
                continue;
            }
            if (actor->plane != nullptr) {
                if (actor->plane != this->player_plane) {
                    Vector3D distance = {
                        actor->plane->x - this->player_plane->x,
                        actor->plane->y - this->player_plane->y,
                        actor->plane->z - this->player_plane->z
                    };
                    if (distance.Length() > RENDER_DISTANCE) {
                        continue; // Skip rendering if the plane is too far away
                    }
                    if (this->show_bbox) {
                        actor->plane->renderPlaneLined();
                        BoudingBox *bb = actor->plane->object->entity->GetBoudingBpx();
                        Vector3D position = {actor->plane->x, actor->plane->y, actor->plane->z};
                        Vector3D orientation = {
                            actor->plane->azimuthf/10.0f + 90,
                            actor->plane->elevationf/10.0f,
                            -actor->plane->twist/10.0f
                        };
                        for (auto vertex: actor->plane->object->entity->vertices) {
                            if (vertex.x == bb->min.x) {
                                Renderer.drawPoint(vertex, {1.0f,0.0f,0.0f}, position, orientation);
                            }
                            if (vertex.x == bb->max.x) {
                                Renderer.drawPoint(vertex, {0.0f,1.0f,0.0f}, position, orientation);
                            }
                            if (vertex.z == bb->min.z) {
                                Renderer.drawPoint(vertex, {1.0f,0.0f,0.0f}, position, orientation);
                            }
                            if (vertex.z == bb->max.z) {
                                Renderer.drawPoint(vertex, {0.0f,1.0f,0.0f}, position, orientation);
                            }
                        }
                        Renderer.renderBBox(position, bb->min, bb->max);
                    } else {
                        actor->plane->Render();
                    }
                }
            } else if (actor->object->entity != nullptr) {
                Vector3D actor_position = {actor->object->position.x, actor->object->position.y, actor->object->position.z};
                Vector3D actor_orientation = {(360.0f - static_cast<float>(actor->object->azymuth) + 90.0f), static_cast<float>(actor->object->pitch), -static_cast<float>(actor->object->roll)};
                Vector3D distance = {
                    actor_position.x - this->player_plane->x,
                    actor_position.y - this->player_plane->y,
                    actor_position.z - this->player_plane->z
                };
                if (distance.Length() > RENDER_DISTANCE) {
                    continue; // Skip rendering if the object is too far away
                }
                Renderer.drawModel(actor->object->entity, Renderer.lodLevel, actor_position, actor_orientation);
                if (this->show_bbox) {
                    if (actor->aiming_vector.x != 0.0f || actor->aiming_vector.y != 0.0f || actor->aiming_vector.z != 0.0f) {
                        Vector3D aim_pos = {actor->aiming_vector.x, actor->aiming_vector.y, actor->aiming_vector.z};
                        Renderer.drawLine(actor_position, aim_pos, {1.0f, 0.0f, 0.0f});
                    }
                }
                for (auto weapons : actor->weapons_shooted) {
                    if (weapons->alive) {
                        weapons->Render();
                    }
                }
                if (this->show_bbox) {
                    BoudingBox *bb = actor->object->entity->GetBoudingBpx();
                    Vector3D position = {actor->object->position.x, actor->object->position.y, actor->object->position.z};
                    Renderer.renderBBox(position, bb->min, bb->max);
                }
            }
        }

        for (auto expl: this->current_mission->explosions) {
            if (expl->is_finished) {
                // Remove explosion when finished
                this->current_mission->explosions.erase(
                    std::remove_if(
                        this->current_mission->explosions.begin(),
                        this->current_mission->explosions.end(),
                        [](const auto& expl) { return expl->is_finished; }
                    ),
                    this->current_mission->explosions.end());
                continue;
            }
            expl->render();
        }

        this->cockpit->cam = camera;

        Vector3D centerPoint = {0,0,-10};
        
        centerPoint = camera->getPosition()+camera->getForward()*10.0f;
        Renderer.drawPoint(centerPoint, {0.0f, 1.0f, 0.0f}, {0,0,0}, {0.0f, 0.0f, 0.0f});
        Renderer.drawPoint(this->cockpit->targetImpactPointWorld, {0.0f, 1.0f, 1.0f}, {0,0,0}, {0.0f, 0.0f, 0.0f});
        this->player_plane->RenderSimulatedObject();
        for (auto objs: this->simulated_objects) {
            if (objs->alive) {
                objs->Simulate(this->player_plane->tps);
                objs->Render();
            }
        }
        switch (this->camera_mode) {
        case View::CAM_DIRECTOR:
        case View::MISSILE_CAM:
        case View::TARGET:
        case View::OBJECT:
        case View::AUTO_PILOT:
        case View::CRASH_VIEW_P3:
        case View::FOLLOW:
            if (!this->show_bbox) {
                this->player_plane->Render();
            } else {
                this->player_plane->renderPlaneLined();
            }
            break;
        case View::FRONT:
            if (!forceVirtualCockpit) {
                if (this->zoom_cockpit) {
                    this->cockpit->Render(CockpitFace::CP_BIG);
                } else {
                    this->cockpit->Render(CockpitFace::CP_FRONT);
                }
                break;
            }
        case View::RIGHT:
            if (!forceVirtualCockpit) {
                this->cockpit->Render(CockpitFace::CP_RIGHT);
                break;    
            }
        case View::LEFT:
            if (!forceVirtualCockpit) {
                this->cockpit->Render(CockpitFace::CP_LEFT);
                break;
            }
        case View::REAR:
            if (!forceVirtualCockpit) {
                this->cockpit->Render(CockpitFace::CP_REAR);
                break;
            }
        case View::CRASH_VIEW_P1:
            if (!forceVirtualCockpit) {
                if (this->cockpit->RenderCrashAnimationP1()) {
                    this->camera_mode = View::CRASH_VIEW_P2;
                    this->cockpit->frame = 0;
                }
                break;
            }
        case View::CRASH_VIEW_P2:
            if (!forceVirtualCockpit) {
                if (this->cockpit->RenderCrashAnimationP2()) {
                    this->camera_mode = View::CRASH_VIEW_P3;
                }
                break;
            }
        case View::EJECT_VIEW_P1:
            if (!forceVirtualCockpit) {
                if (this->cockpit->RenderEjectionAnimation()) {
                    this->camera_mode = View::EJECT_VIEW_P2;
                }
                break;
            }
        case View::EJECT_VIEW_P2:
        case View::EJECT_VIEW_P3:
            this->player_plane->Render();
            break;
        case View::EYE_ON_TARGET:
        case View::REAL:
        case View::CONTROLLER_LOOK:
            this->renderVirtualCockpit();
            break;
        }
    };

    // VR stéréo: rendu par oeil + cockpit VR uniquement.
    bool vrShouldRender = false;
    const uint32_t eyeCount = Screen ? Screen->vrStereoEyeCount() : 0;
    if (eyeCount >= 2 && Screen->vrPrepareStereoFrame(vrShouldRender)) {
        if (!vrShouldRender) {
            Screen->vrEndStereoFrame();
            return;
        }

        const float metersToWorld = 1.0f;
        const float zNear = 1.5f;
        const float zFar = (float)(BLOCK_WIDTH * BLOCK_PER_MAP_SIDE * 4);

        const Point3D camPos = camera->getPosition();
        Quaternion camOri = camera->getOrientation();
        // Camera::orientation encode la rotation de vue (world->camera).
        // Pour OpenXR, on a besoin d'un repère "local" (cockpit) vers le monde.
        // Donc on prend l'inverse de la rotation (transpose pour une rotation pure).
        Matrix worldFromLocal = camOri.ToMatrix();
        worldFromLocal.Transpose();
        worldFromLocal.v[3][0] = camPos.x;
        worldFromLocal.v[3][1] = camPos.y;
        worldFromLocal.v[3][2] = camPos.z;
        worldFromLocal.v[3][3] = 1.0f;

        for (uint32_t eye = 0; eye < eyeCount; ++eye) {
            VRStereoEyeRenderInfo eyeInfo;
            if (!Screen->vrBeginStereoEye(eye, worldFromLocal, metersToWorld, zNear, zFar, eyeInfo)) {
                continue;
            }
            camera->setCustomMatrices(eyeInfo.projection, eyeInfo.view);
            this->cockpit->is_3d_cockpit = true;
            renderScene(eyeInfo.width, eyeInfo.height, 0.0f, true);

			if (this->cockpit && this->cockpit->target) {
				drawTargetSquareOverlay(
					eyeInfo.width,
					eyeInfo.height,
					eyeInfo.projection,
					eyeInfo.view,
					this->cockpit->target->position);
			}
        }

        camera->clearCustomMatrices();
        Screen->vrEndStereoFrame();
        return;
    }

    renderScene(Renderer.width, Renderer.height, verticalOffset, false);
}
