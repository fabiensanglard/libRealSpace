#include "precomp.h"
#define GRAVITY 32.174f
#define DRAG_COEFFICIENT 0.47f  // Coefficient de traînée
#define CROSS_SECTIONAL_AREA 0.1f  // Section transversale (m^2)
#define LIFT_COEFFICIENT  0.5f
#define MAX_VELOCITY 15.0  // Vitesse maximale autorisée en m/s

SCSimulatedObject::SCSimulatedObject() {             

    this->elevationf = 0.0f;
    this->vx = 0.0f;
    this->vy = 0.0f;
    this->vz = 0.0f;

    this->azimuthf = 0.0f;
    this->elevationf = 0.0f;

}
SCSimulatedObject::~SCSimulatedObject() {

}

void SCSimulatedObject::Render() {
    glPushMatrix();
    Matrix rotation;
    rotation.Clear();
    rotation.Identity();
    rotation.translateM(this->x, this->y, this->z);
    rotation.rotateM(this->azimuthf, 0.0f, 1.0f, 0.0f);
    rotation.rotateM(this->elevationf, 0.0f, 0.0f, 1.0f);
    rotation.rotateM(0.0f, 1.0f, 0.0f, 0.0f);
    
    glMultMatrixf((float *)rotation.v);
    
    Renderer.DrawModel(this->obj, LOD_LEVEL_MAX);
    glPopMatrix();
}
void SCSimulatedObject::GetPosition(Vector3D *position) {
    position->x = this->x;
    position->y = this->y;
    position->z = this->z;
}
// Fonction pour convertir les coordonnées polaires en coordonnées cartésiennes
Vector3D polarToCartesian(float speed, float phi, float theta) {
    Vector3D v;
    v.x = speed * sinf(theta) * cosf(phi);
    v.y = speed * sinf(theta) * sinf(phi);
    v.z = speed * cosf(theta);
    return v;
}
#define EPSILON 1e-10 // Tolérance pour les très petits nombres
// Fonction pour convertir les coordonnées cartésiennes en coordonnées polaires
void cartesianToPolar(Vector3D v, float *phi, float *theta) {
    *phi = atan2f(v.x, v.z);
    *theta = acosf(v.y / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z));
}

// Calcul de la norme d'un vecteur

// Calcul de la force de frottement
Vector3D SCSimulatedObject::calculate_drag(Vector3D velocity) {
    float speed = velocity.Norm();
    int itemp;
    itemp = ((int)this->y) >> 10;
    if (itemp > 74) {
        itemp = 74;
    } else if (itemp < 0) {
        itemp = 0;
    }
    float drag_magnitude = 0.5f * ro[itemp] * powf(speed,2) *DRAG_COEFFICIENT * CROSS_SECTIONAL_AREA;
    return velocity * (-drag_magnitude / speed);
}
Vector3D SCSimulatedObject::calculate_lift(Vector3D velocity) {
    float speed = velocity.Norm();
    int itemp;
    itemp = ((int)this->y) >> 10;
    if (itemp > 74) {
        itemp = 74;
    } else if (itemp < 0) {
        itemp = 0;
    }
    float lift_magnitude = 0.5f * ro[itemp] * powf(speed,2) * LIFT_COEFFICIENT * CROSS_SECTIONAL_AREA;
    if (lift_magnitude< this->weight * GRAVITY) {
        lift_magnitude = this->weight * GRAVITY;
    }
    Vector3D lift_direction = {
        0.0, lift_magnitude, 0.0
    };
    return lift_direction;
}
void SCSimulatedObject::SimulateWithVector(int tps) {
    float deltaTime = 1.0f / (float) tps;
    float thrust = 0.0f;
    if (this->obj->dynn_miss != nullptr) {
        thrust = (float)this->obj->dynn_miss->velovity_m_per_sec*3.2808399f;
    }
    Vector3D position = {
        this->x,
        this->y,
        this->z
    };
    Vector3D velocity = {
        this->vx,
        this->vy,
        this->vz
    };
    Vector3D to_target = {
        0.0f,
        0.0f,
        0.0f
    };
    if (this->target != nullptr) {
        to_target.x = (float) this->target->x;
        to_target.y = (float) this->target->z;
        to_target.z = (float) -this->target->y;
    }
    Vector3D gravity_force = {0.0, -this->weight * GRAVITY, 0.0};
    Vector3D drag_force = this->calculate_drag(velocity);
    Vector3D lift_force = this->calculate_lift(velocity);
    
    Vector3D error = (to_target - position);
    
        
    
    if (this->target != nullptr && error.Norm() < 10.0f) {
        this->alive = false;
        this->target->alive = false;
        return;
    }
    // Force de poussée ajustée en fonction de la direction de l'erreur
    Vector3D thrust_force = error * (thrust / error.Norm());

    Vector3D total_force = gravity_force + drag_force + lift_force + thrust_force;
    Vector3D acceleration = total_force * (1.0f/ this->weight);
    velocity = (velocity+(acceleration*deltaTime)).limit(MAX_VELOCITY);
    error.Normalize();
    velocity = (error * thrust)*deltaTime;
    position = position+velocity;
    
    float azimut = 0.0f;
    float elevation = 0.0f;
    cartesianToPolar(velocity, &azimut, &elevation);
    this->azimuthf = (float)(azimut - M_PI_2);
    this->elevationf = (float)(M_PI_2-elevation);
    this->vx = velocity.x;
    this->vy = velocity.y;
    this->vz = velocity.z;
    this->x = position.x;
    this->y = position.y;
    this->z = position.z;
    

}
void SCSimulatedObject::Simulate(int tps) {
    this->SimulateWithVector(tps);
}