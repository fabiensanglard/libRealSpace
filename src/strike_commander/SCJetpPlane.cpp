//
//  SCJetpPlane.cpp
//  libRealSpace
//
//  Implementation de l'algorithme de vol reconstruit depuis STRIKE.EXE.
//  Chaque bloc renvoie a la section correspondante de
//  strike_commander_re/analysis/DATA_MODEL.md (§6.2) et PHYSICS.md.
//
//  Reperes :
//   - Monde  : X/Z horizontal, Y = altitude (convention deja etablie par SCPlane/RSArea).
//   - Corps (local, vx/vy/vz) : x = envergure (ASM c0), y = vertical/portance (ASM c2),
//     z = -nez (ASM c1) ; forward = {0,0,-1} donc "avancer" = vz < 0.
//
#include "precomp.h"
#include "../engine/gametimer.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float G_SI = 9.797f;          // strike.asm:144603, dword_6FFD7 = -2508 (24.8) = -9.797 m/s^2
constexpr float RAD2DEG_57_29 = 57.29578f; // dword_707B2 = 0x394B = 180/pi ; angles d'ecoulement en degres
constexpr float HREF_M = 11000.0f;      // altitude de reference du lapse de poussee

// Zone morte du servo d'attitude : l'ASM compare l'erreur (en 24.8) contre 56 brut,
// soit 56/256 = 0.21875 degre. Valeur physique reelle en degres.
constexpr float ATTITUDE_DEAD_ZONE_DEG = 0.21875f;

// dword_70454 : frequence d'images de l'original, bridee a 25 images/s (pas un gain).
constexpr float ORIGINAL_FPS = 25.0f;
}

SCJetpPlane::SCJetpPlane() {
}

SCJetpPlane::SCJetpPlane(float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE,
                          float s, float W, float fuel_weight, float Mthrust, float b, float ie_pi_AR,
                          int MIN_LIFT_SPEED, RSArea *area, float x, float y, float z)
    : SCPlane(LmaxDEF, LminDEF, Fmax, Smax, ELEVF_CSTE, ROLLFF_CSTE, s, W, fuel_weight, Mthrust, b, ie_pi_AR,
              MIN_LIFT_SPEED, area, x, y, z) {
    // Valeurs de repli tant que object->entity->jdyn n'est pas encore attache par l'appelant
    // (loadFromEntity() les remplace par les vraies donnees JDYN/DYNM/THRS/STBL des que possible).
    this->mass_kg = W;
    this->empty_mass_kg = W;
    this->thrust_max_n = Mthrust;
    this->fuel_capacity_kg = fuel_weight;
    this->fuel_kg = fuel_weight;
    this->fuel = (int)this->fuel_kg;
    this->inverse_mass = (this->mass_kg > 0.0f) ? (1.0f / this->mass_kg) : 0.0f;
    // position/velocity sont l'etat maitre depuis updatePosition() : il faut les amorcer avec les
    // coordonnees de spawn, sinon ils restent a (0,0,0) (sous le sol -> crash immediat au chargement).
    this->position = Vector3D(x, y, z);
    this->velocity = Vector3D(0.0f, 0.0f, 0.0f);
    this->ptw.Clear();
    this->incremental.Clear();
}

SCJetpPlane::~SCJetpPlane() {
}

void SCJetpPlane::loadFromEntity() {
    if (this->entity_loaded) {
        return;
    }
    if (this->object == nullptr || this->object->entity == nullptr) {
        return;
    }

    RSEntity *entity = this->object->entity;

    this->empty_mass_kg = (float)entity->weight_in_kg;
    this->mass_kg = this->empty_mass_kg;
    this->thrust_max_n = (float)entity->thrust_in_newton;
    this->thrust_mil_fraction = entity->thrust_mil_fraction;
    this->thrust_ref_alt_fraction = entity->thrust_ref_alt_fraction;
    this->thrust_cutoff_alt_m = entity->thrust_cutoff_alt_raw * 100.0f;
    this->stability_gain = entity->stability_gain;
    this->parasite_drag = entity->parasite_drag;

    JDYN *jdyn = entity->jdyn;
    if (jdyn != nullptr) {
        this->fuel_capacity_kg = (float)jdyn->fuel_capacity;
        this->sfc = jdyn->sfc;
        this->drag_airbrake = jdyn->drag_airbrake;
        this->drag_gear = jdyn->drag_gear;
        this->ground_moment_1 = jdyn->ground_moment_1;
        this->ground_moment_2 = jdyn->ground_moment_2;
        this->rate_limit_dps = jdyn->rate_limit_dps;
        this->max_turn_rate_dps = jdyn->max_turn_rate_dps;
        this->stall_alpha_deg = (float)jdyn->stall_alpha_deg;
        this->wing_incidence_deg = (float)jdyn->wing_incidence_deg;
        this->flap_lift_increment_deg = (float)jdyn->flap_lift_increment_deg;
        this->pitch_rate_limit_dps = (float)jdyn->pitch_rate_limit_dps;
        this->control_speed_ms = jdyn->control_speed_ms;
        this->induced_drag_k = jdyn->induced_drag_k;
        this->lift_gain = jdyn->lift_gain;
        this->yaw_authority = (float)jdyn->yaw_authority; 
        this->pitch_stick_gain = (float)jdyn->pitch_stick_gain;
        this->pitch_load_gain = (float)jdyn->max_g;
    }

    this->fuel_kg = this->fuel_capacity_kg;
    this->fuel = (int)this->fuel_kg;
    this->updateMass();
    this->Mthrust = this->thrust_max_n;
    this->entity_loaded = true;

    if (!this->airdens.isLoaded()) {
        TreEntry *airdensEntry = AssetManager::getInstance().GetEntryByName("..\\..\\DATA\\AIRDENS.TBL");
        if (airdensEntry != nullptr) {
            this->airdens.initFromRam(airdensEntry->data, airdensEntry->size);
        }
    }

    printf(
        "SCJetpPlane::loadFromEntity jdyn=%p mass_kg=%.2f thrust_max_n=%.2f stability_gain=%.4f parasite_drag=%.4f\n",
        (void *)(this->object ? this->object->entity->jdyn : nullptr),
        this->mass_kg,
        this->thrust_max_n,
        this->stability_gain,
        this->parasite_drag
    );
    printf(
        "  wing_incidence_deg=%.2f flap_lift_increment_deg=%.2f stall_alpha_deg=%.2f\n",
        this->wing_incidence_deg,
        this->flap_lift_increment_deg,
        this->stall_alpha_deg
    );
    printf(
        "  lift_gain=%.4f induced_drag_k=%.4f yaw_authority=%.2f\n",
        this->lift_gain,
        this->induced_drag_k,
        this->yaw_authority
    );
    printf(
        "  pitch_stick_gain(jdyn65)=%.2f pitch_load_gain(jdyn67)=%.2f rate_limit_dps=%.2f max_turn_rate_dps=%.2f "
        "pitch_rate_limit_dps(champ15)=%.2f\n",
        this->pitch_stick_gain,
        this->pitch_load_gain,
        this->rate_limit_dps,
        this->max_turn_rate_dps,
        this->pitch_rate_limit_dps
    );
}

float SCJetpPlane::airDensity(float altitude_m) const {
    return this->airdens.densityAt(altitude_m);
}

float SCJetpPlane::throttleNotchToThrustFraction(float notch) const {
    // Aero_ThrottleThrustCurve : cran 0-10 (MIL 0-5 / PC 5-10).
    if (notch <= 0.0f) {
        return 0.0f;
    }
    if (notch <= 5.0f) {
        return (notch / 5.0f) * this->thrust_mil_fraction;
    }
    return this->thrust_mil_fraction + (1.0f - this->thrust_mil_fraction) * (notch - 5.0f) / 5.0f;
}

float SCJetpPlane::altitudeLapseFraction(float altitude_m) const {
    if (altitude_m <= HREF_M) {
        return 1.0f - (altitude_m / HREF_M) * (1.0f - this->thrust_ref_alt_fraction);
    }
        
    float denom = this->thrust_cutoff_alt_m - HREF_M;
    if (denom <= 0.0f) {
        return 0.0f;
    }
    float f = this->thrust_ref_alt_fraction - this->thrust_ref_alt_fraction * (altitude_m - HREF_M) / denom;
    return f > 0.0f ? f : 0.0f;
}

void SCJetpPlane::computeGravity() {
    this->gravity = G_SI;
    this->gravity_force = this->mass_kg * G_SI;
}

void SCJetpPlane::computeThrust() {
    int max_notch = (int) lroundf(10.0f * this->g_engine);
    if (this->thrust > max_notch * 10) {
        this->thrust = max_notch * 10;
    }
    float notch = this->thrust / 10.0f;
    float fraction = this->throttleNotchToThrustFraction(notch) * this->altitudeLapseFraction(this->y);
    this->thrust_force = this->thrust_max_n * fraction * this->g_engine;
    if (this->fuel_kg <= 0.0f) {
        this->thrust_force = 0.0f;
    }
    this->thrust_vector = Vector3D(0.0f, 0.0f, -this->thrust_force);
}

void SCJetpPlane::computeLift() {
    float V = sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);
    if (V < 0.5f) {
        this->alpha_deg = this->beta_deg = this->alpha_eff_deg = 0.0f;
        this->ae = 0.0f;
        this->dynamic_pressure = 0.0f;
        this->lift_force = 0.0f;
        this->lift_vector = Vector3D(0.0f, 0.0f, 0.0f);
        this->side_vector = Vector3D(0.0f, 0.0f, 0.0f);
        this->wing_stall = 0;
        return;
    }

    this->alpha_deg = -RAD2DEG_57_29 * (this->vy / V);
    this->beta_deg = -RAD2DEG_57_29 * (this->vx / V);

    float ae_raw = this->alpha_deg + this->wing_incidence_deg;
    if (this->flaps > 0) {
        ae_raw += this->flap_lift_increment_deg;
    }

    // Decrochage -- Aero_ComputeLiftAndSideForce (DATA_MODEL.md §6.2, PHYSICS.md §5.2).
    // (A) SATURATION, toujours active : alpha_eff borne symetriquement AVANT le calcul de portance.
    //     Au-dela du seuil la portance PLAFONNE (pas de courbe post-decrochage).
    this->alpha_eff_deg = (std::clamp)(ae_raw, -this->stall_alpha_deg, this->stall_alpha_deg);
    //     La force laterale est coupee net si le derapage depasse le meme seuil (loc_482E7).
    bool side_stall = fabsf(this->beta_deg) > this->stall_alpha_deg;
    // (B) DEPART FRANC : portance forcee a zero (loc_481CF). Avion du joueur uniquement, sous option
    //     de realisme (regroupe word_70466 > 10 ET byte_72354 de l'ASM, cf. realistic_stall).
    bool hard_stall = fabsf(ae_raw) > this->stall_alpha_deg && this->realistic_stall &&
                      this->pilot != nullptr && this->pilot->actor_name == "PLAYER";

    this->wing_stall = hard_stall ? 1 : 0; // proxy de flags_75.bit6 (alerte) : seulement le depart franc
    this->ae = this->alpha_eff_deg;

    this->dynamic_pressure = 0.5f * this->airDensity(this->y) * V * V;

    // Portance + force laterale : Aero_ComputeLiftAndSideForce (DATA_MODEL.md §6.2).
    float k_lift = this->lift_gain * this->g_wing * this->dynamic_pressure;
    this->lift_force = hard_stall ? 0.0f : k_lift * this->alpha_eff_deg;
    Vector3D dirLift(0.0f, -this->vz, this->vy); // perpendiculaire a la vitesse, plan vertical corps
    dirLift.Normalize();
    this->lift_vector = dirLift * this->lift_force;

    float k_side = 0.25f * this->lift_gain * this->dynamic_pressure;
    float sideForce = side_stall ? 0.0f : k_side * this->beta_deg;
    Vector3D dirSide(-this->vz, 0.0f, this->vx); // perpendiculaire a la vitesse, plan horizontal corps
    dirSide.Normalize();
    this->side_vector = dirSide * sideForce;

    this->Lmax = this->stall_alpha_deg;
    this->Lmin = -this->stall_alpha_deg;
}

void SCJetpPlane::computeDrag() {
    bool airbrakeOut = this->spoilers > 0;
    bool gearOut = this->wheels != 0;

    // Traînée : Aero_ComputeDragWithFeedback (DATA_MODEL.md §6.2).
    this->Cd = this->induced_drag_k * this->alpha_eff_deg * this->alpha_eff_deg +
               (this->induced_drag_k * 0.125f) * this->beta_deg * this->beta_deg +
               (1.0f + (airbrakeOut ? this->drag_airbrake : 0.0f) + (gearOut ? this->drag_gear : 0.0f)) * this->parasite_drag;

    this->drag_force = this->dynamic_pressure * this->Cd;

    if (this->on_ground) {
        float groundTerm = this->ground_moment_1 + (airbrakeOut ? this->ground_moment_2 : 0.0f);
        float forwardSpeed = -this->vz; // ASM c1 (nez) = -vz local, cf. DATA_MODEL.md §6.2
        if (fabsf(forwardSpeed) < 1.0f) {
            groundTerm *= forwardSpeed;
        }
        this->drag_force += 256.0f * groundTerm;
    }

    float V = sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);
    if (V > 0.01f) {
        Vector3D dirDrag(this->vx / V, this->vy / V, this->vz / V);
        this->drag_vector = dirDrag * (-this->drag_force);
    } else {
        this->drag_vector = Vector3D(0.0f, 0.0f, 0.0f);
    }
}

void SCJetpPlane::updateForces() {
    this->acceleration.x = this->lift_vector.x + this->side_vector.x + this->drag_vector.x + this->thrust_vector.x;
    this->acceleration.y = this->lift_vector.y + this->side_vector.y + this->drag_vector.y + this->thrust_vector.y;
    this->acceleration.z = this->lift_vector.z + this->side_vector.z + this->drag_vector.z + this->thrust_vector.z;
}

void SCJetpPlane::updateAcceleration() {
    this->acceleration.x *= this->inverse_mass;
    this->acceleration.y *= this->inverse_mass;
    this->acceleration.z *= this->inverse_mass;

    // Gravite (monde) projetee en repere corps via ptw (deja reconstruite pour ce tic par
    // updatePosition(), qui s'execute avant dans Simulate() - simple lecture immediate, pas un
    // etat qu'on relit d'un tic sur l'autre), ajoutee APRES la division par la masse
    // (Aero_SumLinearForces_48639).
    this->gravity_vector = Vector3D(
        -this->ptw.v[0][1] * this->gravity,
        -this->ptw.v[1][1] * this->gravity,
        -this->ptw.v[2][1] * this->gravity
    );
    this->acceleration.x += this->gravity_vector.x;
    this->acceleration.y += this->gravity_vector.y;
    this->acceleration.z += this->gravity_vector.z;
}

void SCJetpPlane::updateVelocity() {
    float dt = GameTimer::getInstance().getDeltaTime();
    if (dt <= 0.0f)
        dt = 1.0f / 30.0f;

    // this->acceleration est en repere CORPS a ce stade. Contrainte sol appliquee dans ce repere,
    // comme dans PhysicsTicks (projection de la vitesse hors du plan sol).
    if (this->on_ground && this->status > MEXPLODE) {
        this->acceleration.x = 0.0f;
        float sinPitch = sinf(tenthOfDegreeToRad(this->pitch));
        float cosPitch = cosf(tenthOfDegreeToRad(this->pitch));
        if (cosPitch == 0.0f) {
            cosPitch = 0.0001f;
        }
        float floorVy = this->vz * sinPitch / cosPitch;
        if (this->vy + this->acceleration.y * dt < floorVy) {
            this->acceleration.y = (floorVy - this->vy) / dt;
        }
    }

    // Transform corps->monde pour la partie LINEAIRE (Physics_IntegratePosition, seg101).
    // IMPORTANT : on utilise EXACTEMENT la meme matrice `ptw` que updatePosition() pour le
    // transform inverse monde->corps. Reconstruire une matrice depuis les angles d'Euler
    // re-derives ferait diverger les deux transforms (la re-derivation n'est pas l'inverse exact
    // de la composition incrementale) -> l'aller-retour corps<->monde ne serait plus l'identite
    // -> vitesse qui s'emballe, traînee et G delirants (bug constate le 2026-09-05).
    Matrix rot = this->ptw;
    rot.v[3][0] = rot.v[3][1] = rot.v[3][2] = 0.0f; // partie rotation seule
    Vector3D worldAccel = this->acceleration.transformPoint(rot);
    this->velocity += worldAccel * dt;
}

void SCJetpPlane::updatePosition() {
    // ORIENTATION = matrice persistante tournee par petits increments (mecanisme ASM decode le
    // 2026-09-05, voir DATA_MODEL.md §PhysicsTicks / known_functions.json) :
    //   WorldObject_ComposeOrientation3Angles_3CAE3 (methode vtable des objets du monde, famille
    //   ~18 vtables) -> Matrix_BuildFullOrientation_575B2(objet+0x2C, &aX, &aY, &aZ)
    //     -> Matrix_BuildAxisX/Y/Z_56EC3 : chacun applique une rotation d'axe INCREMENTALE EN PLACE
    //        sur les lignes de la matrice d'orientation PERSISTANTE (rotation standard :
    //        ligne0' = c*ligne0 - s*ligne2, ligne2' = c*ligne2 + s*ligne0), sans reconstruction,
    //        no-op si |angle| < 0x38 brut = 56/256 = 0.21875 deg. L'integration est faite par
    //        WorldObject_IntegrateBodyMotion_3D31D (methode +0x14), qui fait aussi position += v*dt.
    // Les 3 angles sont les INCREMENTS = vitesses angulaires physique * dt. Il n'y a donc PAS
    // d'accumulateur d'angle d'Euler cote jeu d'origine : l'etat d'orientation vit uniquement dans
    // la matrice. `pitch`/`yaw`/`roll` (SCPlane) sont ici RE-DERIVES de la matrice a chaque tic pour
    // le reste du moteur (HUD, rendu) ; ils ne sont l'entree que lorsqu'ils sont ecrits de
    // l'exterieur (heading de spawn) - detecte plus bas.
    float dt = GameTimer::getInstance().getDeltaTime();

    this->last_px = this->x;
    this->last_py = this->y;
    this->last_pz = this->z;

    // --- Re-seed de la position si x/y/z ont ete ecrits de l'exterieur (autopilote, teleport,
    //     "set altitude", spawn...) ou au 1er tic. `position` (Vector3D) est l'etat maitre de
    //     l'integration ; on la resynchronise depuis x/y/z quand ils divergent. ---
    if (!this->position_seeded || this->x != this->m_seed_x || this->y != this->m_seed_y ||
        this->z != this->m_seed_z) {
        this->position = Vector3D(this->x, this->y, this->z);
        this->position_seeded = true;
    }

    // --- Integration lineaire (Physics_IntegratePosition) ---
    this->position += this->velocity * dt;

    // --- Re-seed de la matrice si un angle a ete ecrit de l'exterieur (spawn : heading) ou au 1er tic ---
    bool eulerDirty = !this->orientation_seeded || this->pitch != this->m_seed_pitch || this->yaw != this->m_seed_yaw || this->roll != this->m_seed_roll;
    if (eulerDirty) {
        Matrix seed;
        seed.Identity();
        seed.rotateM(tenthOfDegreeToRad(this->yaw), 0, 1, 0);
        seed.rotateM(tenthOfDegreeToRad(this->pitch), 1, 0, 0);
        seed.rotateM(tenthOfDegreeToRad(this->roll), 0, 0, 1);
        this->ptw = seed;
        this->orientation_seeded = true;
    }

    // --- Composition INCREMENTALE en repere avion (Matrix_BuildFullOrientation_575B2 : X puis Y puis Z) ---
    // pitch_speed/yaw_speed/roll_speed sont en deg/s (issus du servo aero, processInput). L'increment
    // de rotation de ce tic = vitesse_angulaire * dt, converti en radians. PAS de *10 : ce facteur
    // venait de l'ancien accumulateur d'Euler en dixiemes de degre, il n'a plus lieu d'etre ici.
    float dPitch = degreeToRad(this->pitch_speed * dt);
    float dYaw = degreeToRad(this->yaw_speed * dt);
    float dRoll = degreeToRad(this->roll_speed * dt);
    // NB : le seuil "0x38" (0.21875 deg) de Matrix_BuildAxis*_56EC3 est une micro-optim ASM
    // "skip si negligeable", calibree pour le tick DOS a taux fixe (~15-30 Hz). A framerate
    // variable/eleve, l'increment par tic passe sous ce seuil et l'avion ne tournerait jamais :
    // on ne le reproduit PAS ici.
    if (dPitch != 0.0f) {
        this->ptw.rotateM(dPitch, 1, 0, 0);
    }
    if (dYaw != 0.0f) {
        this->ptw.rotateM(dYaw, 0, 1, 0);
    }
    if (dRoll != 0.0f) {
        this->ptw.rotateM(dRoll, 0, 0, 1);
    }

    // --- Re-derivation des angles d'Euler depuis la matrice, pour le reste du moteur ---
    // Convention de build : R = Rz(roll) * Rx(pitch) * Ry(yaw) (ordre des rotateM ci-dessus).
    // Lignes de ptw = axes locaux X/Y/Z exprimes en monde (convention vecteur-ligne de Vector3D).
    Matrix &m = this->ptw;
    float sinPitch = (std::clamp)(-m.v[2][1], -1.0f, 1.0f);
    float newPitchDeg = radToDegree(asinf(sinPitch));
    float newYawDeg = radToDegree(atan2f(m.v[2][0], m.v[2][2]));
    float newRollDeg = radToDegree(atan2f(m.v[0][1], m.v[1][1]));

    this->m_old_pitch = this->pitch;
    this->m_old_yaw = this->yaw;
    this->pitch = newPitchDeg * 10.0f;
    this->yaw = norm3600(newYawDeg * 10.0f);
    this->roll = norm3600(newRollDeg * 10.0f);
    this->m_pitch_var = this->pitch - this->m_old_pitch;
    this->m_yaw_var = this->yaw - this->m_old_yaw;
    this->m_seed_pitch = this->pitch;
    this->m_seed_yaw = this->yaw;
    this->m_seed_roll = this->roll;

    // --- Sorties derivees pour le reste du moteur ---
    this->x = this->position.x;
    this->y = this->position.y;
    this->z = this->position.z;
    this->m_seed_x = this->x;   // memorise pour detecter une ecriture externe au prochain tic
    this->m_seed_y = this->y;
    this->m_seed_z = this->z;
    this->groundlevel = this->area->getY(this->x, this->z);

    this->ptw.SetTranslation(this->x, this->y, this->z);
    this->forward = Vector3D(-this->ptw.v[2][0], -this->ptw.v[2][1], -this->ptw.v[2][2]);

    Matrix worldToBody = this->ptw;
    worldToBody.v[3][0] = worldToBody.v[3][1] = worldToBody.v[3][2] = 0.0f;
    worldToBody.Transpose();
    Vector3D bodyVelocity = this->velocity.transformPoint(worldToBody);
    this->vx = bodyVelocity.x;
    this->vy = bodyVelocity.y;
    this->vz = bodyVelocity.z;

    // Meme convention que SCJdynPlane (lue par le pipper du canon) : rotation par tick, lissee.
    const float alpha = 0.1f;
    Vector3D rawAngularVelocity(-dPitch, -dYaw, -dRoll);
    this->angular_velocity = rawAngularVelocity * alpha + this->angular_velocity * (1.0f - alpha);
}

void SCJetpPlane::processInput() {
    float dt = GameTimer::getInstance().getDeltaTime();
    
    // control_stick_x/y = offset en pixels, pleine deflexion = hauteur/largeur d'ecran / 2.5
    // (souris, SCStrike.cpp L1496/1500) ou +/-150 / +/-200 (clavier). L'ASM travaille avec un
    // manche normalise +/-1.0 (24.8 : [ctrl+0x1F] pleine butee +/-16.0, puis /16). On normalise
    // donc sur la MEME reference que la souris (hauteur/2.5 ~ hauteur/2 cote Remi) et on borne
    // +/-1 pour que le clavier (150) reste utilisable meme s'il sature un peu tot.
    float refY = 150.0f, refX = 200.0f;
    if (RSScreen::hasInstance()) {
        RSScreen &scr = RSScreen::instance();
        if (scr.height > 2) refY = scr.height / 2.5f;   // meme reference que la souris (SCStrike L1500)
        if (scr.width > 2) refX = scr.width / 2.5f;
    }
    // Signe conserve de l'implementation validee par Remi ("comportement des axes bon") :
    // elevator NEGATIF quand on tire le manche.
    this->elevator = (std::clamp)(-(this->control_stick_y / refY), -1.0f, 1.0f);
    this->rollers = (std::clamp)(-(this->control_stick_x / refX), -1.0f, 1.0f);
    if (this->stick_normalized) {
        this->elevator = (std::clamp)(-this->stick_norm_y, -1.0f, 1.0f);
        this->rollers = (std::clamp)(-this->stick_norm_x, -1.0f, 1.0f);
    }

    float V = sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);

    // q = pression dynamique (Aero_DynamicPressure). q' = q * stability_gain / 100 (servo).
    float q = this->dynamic_pressure;
    float qServo = q * this->stability_gain / 100.0f;
    if (qServo < 0.0f) {
        qServo = 0.0f;
    }

    // TANGAGE et LACET : l'ASM (Aero_ComputeAoACommand_48862) ne gate PAS sur on_ground -
    // seulement sur flags_75.bit5 ou q trop faible (pas d'air). Il FAUT l'autorite de tangage au
    // sol pour cabrer au decollage. Le seul gate est "assez de pression dynamique".
    bool ctrlLive = (q > 1.0f);
    // ROULIS : Aero_ComputeControlFlags75Bit5C gate sur vitesse (V <= 0x2800/256 = 40 -> roulis nul)
    // ET pas d'autorite au sol.
    bool rollLive = (!this->on_ground && V > 40.0f);

    // ===================================================================================
    // TANGAGE : Aero_ComputeAoACommand_48862 (consigne d'alpha) + Aero_ComputeForcesMain (servo)
    // Transcription fidele du seg103 L1155-1529 + seg102.
    // Echelle manche : l'ASM fait `loadDemand = MAX_G * [ctrl+0x1F] / 16`. `[ctrl+0x1F]` est un
    // 24.8 fixe de pleine butee +/-16.0 (le code lit sa partie entiere par `sar eax,8`). Le `/16`
    // ramene donc le manche a +/-1.0 -> avec `elevator` deja normalise +/-1 ici, le `/16` ET
    // l'echelle manche s'annulent : loadDemand = MAX_G * elevator.
    // Signe : `elevator` < 0 quand on tire le manche -> demande de charge > 0 -> on utilise -elevator.
    // ===================================================================================
    float loadGain = this->pitch_load_gain;          // MAX_G (jdyn[0x67], octet brut = G)
    if (this->elevator > 0.0f) {                        // pousser (nez bas) : autorite /3 (ASM : var_E < 0)
        loadGain /= 3.0f;
    }
    float loadDemand = -this->elevator * loadGain;    // -elevator : tirer -> demande positive

    // var_30 = cos(tangage du nez) ; signe inverse si sur le dos.
    float bankTerm = cosf(tenthOfDegreeToRad(this->pitch));
    if (this->ptw.v[1][1] < 0.0f) {
        bankTerm = -bankTerm;
    }

    float var38 = loadDemand + bankTerm;

    // incidencePerG = ((mass / q) / lift_gain) * 1.5 * g   (ASM : *0x180=1.5, *dword_6FFD7=g, puis neg)
    float incidencePerG = 0.0f;
    if (q > 0.0f && this->lift_gain != 0.0f) {
        incidencePerG = ((this->mass_kg / q) / this->lift_gain) * 1.5f * G_SI;
    }
    float baseline = -this->wing_incidence_deg;
    if (this->flaps > 0) {
        baseline -= this->flap_lift_increment_deg;
    }

    float boundA = 0.0f, boundB = 0.0f;
    if (var38 != 0.0f) {
        boundA = var38 * incidencePerG;      // ASM : var_2C = var_38 * incidencePerG (si var_38 != 0)
        boundB = bankTerm * incidencePerG;   // ASM : var_28 = var_30 * incidencePerG
    }
    boundA += baseline;                       // ASM loc_48B36 : var_2C += var_22 ; var_28 += var_22
    boundB += baseline;

    // ASM loc_48967..loc_48B42 (seg103 L1284-1518), retrace ligne par ligne.
    // var_1A NE part PAS de 0 : quand flags_75.bit4 est pose (defaut de A5620 = bit4|bit7) et
    // que l'avion est en vol, var_1A = alpha courant (scale par un terme geometrique
    // AI_ComputeGeometrySolution_57C67 si alpha<0 - non decode, approxime a 1 ici). C'est ca qui
    // rend le manche neutre STABLE : boundA==boundB en neutre -> le clamp laisse var_1A = alpha
    // -> err = alpha - alpha = 0 -> aucune action. (Avec var_1A=0 comme avant, err = -alpha ->
    // le servo poussait alpha vers 0 -> alpha_eff = calage_aile -> ~2.2G sans manche.)
    float pitchCommandDeg = 0.0f;
    if (!this->on_ground && !this->wing_stall) {
        pitchCommandDeg = this->alpha_deg;
    }
    // Clamp loc_48B42 : ramene var_1A vers boundA SAUF si var_1A est deja dans l'intervalle
    // [0..boundB] (borne = boundA sinon boundB selon les tests jle/jg/jge exacts).
    
    float v = pitchCommandDeg;
    bool keep;
    if (v <= boundB) {
        keep = (boundA >= v) && (boundA <= boundB);
    } else {
        keep = (boundA <= v) && (boundA >= boundB);
    }
    if (!keep) {
        pitchCommandDeg = boundA;
    }
    
    // clamp final +/- pitch_stick_gain (jdyn[0x65])
    pitchCommandDeg = (std::clamp)(pitchCommandDeg, -this->pitch_stick_gain * this->g_elevator, this->pitch_stick_gain * this->g_elevator);
    this->alpha_command_deg = pitchCommandDeg;

    // --- Servo Aero_ComputeForcesMain (seg102) + Physics_IntegrateSecondaryPosition ---
    // Relu octet-pres (seg102 L2440-2652, seg112 L999-1045) :
    //   target = +/- min( 2*sqrt(q'*|err|) , K*|err| )         K = dword_70454 = ORIGINAL_FPS
    //   accel  = clamp( (target - rate) * K , +/- 3*q' )       [deg/s^2]
    //   rate  += accel * dt

    float errPitch = pitchCommandDeg - this->alpha_deg;
    float targetPitchRate = 0.0f;
    if (ctrlLive && fabsf(errPitch) >= ATTITUDE_DEAD_ZONE_DEG) {
        float sqrtLaw = 2.0f * sqrtf(qServo * fabsf(errPitch));
        targetPitchRate = copysignf((std::min)(sqrtLaw, ORIGINAL_FPS * fabsf(errPitch)), errPitch);
    }
    float servoStep = (std::min)(ORIGINAL_FPS * dt, 1.0f);
    float maxPitchAccel = 3.0f * qServo * dt;   // 3*q' [deg/s^2] integre sur dt
    this->pitch_speed += (std::clamp)((targetPitchRate - this->pitch_speed) * servoStep, -maxPitchAccel, maxPitchAccel);
    this->pitch_speed = (std::clamp)(this->pitch_speed, -this->max_turn_rate_dps, this->max_turn_rate_dps);

    
    float horizontalSpeed = sqrtf(this->velocity.x * this->velocity.x + this->velocity.z * this->velocity.z);
    float gammaDeg = RAD2DEG_57_29 * atan2f(this->velocity.y, horizontalSpeed);
    // ===================================================================================
    // LACET : servo fidele Aero_ComputeForcesMain, err = consigne_palonnier - beta.
    //   - consigne (Aero_ResetAccumulatorFlags75Bit5) = (yaw_authority) * (palonnier/16), PAS de beta.
    //     L'ASM normalise le palonnier a +/-1 (var_4/16, pleine butee 24.8 = +/-16). Ici this->rudder
    //     est en plage +/-10 (clavier, SCStrike.cpp:992) -> on divise par la meme pleine butee.
    //   - beta (Aero_FlowAngle_Sideslip_46AB5) = -Ca * v_corps.c0 / |v| = -Ca * vx / V. C'EST
    //     dans l'ASM : c'est la stabilite de girouette (ramene le derapage vers 0).
    // Signe : le monde ASM est Z-up, le port Y-up avec c1/c2 echanges -> le sens de la rotation
    // de lacet est inverse. On NEGATE la sortie du servo pour retablir la chiralite (sinon
    // beta<0 -> nez tourne du mauvais cote -> derapage amplifie -> vrille divergente).
    // ===================================================================================
    float yawCommandDeg = (this->rudder / 10.0f) * this->yaw_authority * this->g_rudder;
    float errYaw = yawCommandDeg - this->beta_deg;
    float targetYawRate = 0.0f;
    if (ctrlLive && fabsf(errYaw) >= ATTITUDE_DEAD_ZONE_DEG) {
        float sqrtLaw = 2.0f * sqrtf(qServo * fabsf(errYaw));
        targetYawRate = copysignf((std::min)(sqrtLaw, ORIGINAL_FPS * fabsf(errYaw)), errYaw);
    }
    targetYawRate = -targetYawRate;   // chiralite Z-up (ASM) -> Y-up (port)
    float maxYawAccel = 3.0f * qServo * dt;
    this->yaw_speed += (std::clamp)((targetYawRate - this->yaw_speed) * servoStep, -maxYawAccel, maxYawAccel);
    this->yaw_speed = (std::clamp)(this->yaw_speed, -this->max_turn_rate_dps, this->max_turn_rate_dps);
    if (this->on_ground && V < 40.0f) {
        this->yaw_speed = this->rollers * V / 4.0f;
    } else if (this->on_ground) {
        this->yaw_speed = 0.0f;
    }

    // ===================================================================================
    // ROULIS : Aero_ComputeControlFlags75Bit5C - loi directe manche, pas de couplage.
    // ===================================================================================
    float rollRateMax = this->maxRollRate() * this->g_aileron;
    float rollRateTarget = 0.0f;
    if (rollLive) {
        rollRateTarget = this->rollers * rollRateMax;
    }
    rollRateTarget = (std::clamp)(rollRateTarget, -rollRateMax, rollRateMax);

    float maxRollDelta = this->rate_limit_dps * dt;
    this->roll_speed += (std::clamp)(rollRateTarget - (float)this->roll_speed, -maxRollDelta, maxRollDelta);

    if (this->on_ground) {
        this->roll_speed = 0;
    }

    this->elevation_speedf = this->pitch_speed;
    this->azimuth_speedf = this->yaw_speed;
}

void SCJetpPlane::updateSpeedOfSound() {
    // Aucune correspondance ASM trouvee pour Mach/vitesse du son (PHYSICS.md §8) :
    // approximation standard, decorrelee du modele de force decode.
    float sos = 340.3f - 0.0038f * this->y;
    if (sos < 295.0f) {
        sos = 295.0f;
    }
    this->sos = sos;
    float V = sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);
    this->mach = (this->sos > 0.0f) ? (V / this->sos) : 0.0f;
}

void SCJetpPlane::checkStatus() {
    if (this->y > 20000.0f)
        this->thrust_force = 0.0f;

    if (this->y > this->groundlevel + 1.0f) {
        if (!this->takeoff) {
            this->takeoff = true;
            this->landed = false;
        }
        this->on_ground = FALSE;
    } else if (this->y <= this->groundlevel + 0.5f) {
        if (this->object->alive == 0)
            this->status = MEXPLODE;

        if (this->isOnRunWay()) {
            if (!this->on_ground) {
                int rating = report_card(-this->climbspeed, this->roll, (int)this->vx, (int)(-this->vz * 1.944f), this->wheels);
                if (this->nocrash) {
                    if (rating == -1) {
                        this->status = MEXPLODE;
                    } else {
                        this->fuel_kg = (std::min)(this->fuel_kg + (float)rating, this->fuel_capacity_kg);
                        this->fuel = (int)this->fuel_kg;
                    }
                }
            } else if (this->nocrash == 0) {
                this->status = MEXPLODE;
            }
            if (fabsf(this->vz) < 5.0f && this->thrust < 20) {
                this->thrust = 0;
                this->vx = this->vy = this->vz = 0.0f;
                if (this->takeoff)
                    this->landed = true;
            }
        } else {
            this->velocity = Vector3D(0.0f, 0.0f, 0.0f);
            this->vx = this->vy = this->vz = 0.0f;
            this->crached = true;
            this->object->alive = 0;
        }
        this->on_ground = TRUE;
        if (this->status > MEXPLODE) {
            if (this->pitch < 0)
                this->pitch = 0;
            if (this->roll != 0)
                this->roll = 0;
        }
    }

    if (this->fuel_kg <= 0.0f) {
        this->fuel_kg = 0.0f;
        this->fuel = 0;
        this->thrust = 0;
    }
}

void SCJetpPlane::syncKinematicVelocity(float dt) {
    this->vx = this->velocity.x * this->ptw.v[0][0] + this->velocity.y * this->ptw.v[0][1] + this->velocity.z * this->ptw.v[0][2];
    this->vy = this->velocity.x * this->ptw.v[1][0] + this->velocity.y * this->ptw.v[1][1] + this->velocity.z * this->ptw.v[1][2];
    this->vz = this->velocity.x * this->ptw.v[2][0] + this->velocity.y * this->ptw.v[2][1] + this->velocity.z * this->ptw.v[2][2];
}

void SCJetpPlane::onPlaneWreck(const PlaneWreckEvent &event) {
    if (event.plane != this) {
        return;
    }
    SCPlane::onPlaneWreck(event);
    this->lift_gain = 0.0f;
}

void SCJetpPlane::updatePlaneStatus() {
    float V = sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);
    this->airspeed = (int)(V * 1.944f); // m/s -> noeuds, pour les instruments existants
    this->climbspeed = (short)this->vy; // m/s
    this->g_load = (this->lift_force * this->inverse_mass) / G_SI;
    this->ax = this->acceleration.x;
    this->ay = this->acceleration.y;
    this->az = this->acceleration.z;
}

void SCJetpPlane::Simulate() {
    if (!this->entity_loaded)
        this->loadFromEntity();
    this->updateDamageGains();
    this->updateMass();

    float dt = GameTimer::getInstance().getDeltaTime();
    if (dt <= 0.0f)
        dt = 1.0f / 30.0f;
    this->tps = (uint32_t)(1.0f / dt + 0.5f);
    this->fps_knots = this->tps * (3600.0f / 6082.0f);

    if (this->chaff_timer > 0)
        this->chaff_timer -= dt;
    if (this->flare_timer > 0)
        this->flare_timer -= dt;
    this->updateWeaponTimers(dt);

    this->groundlevel = this->area->getY(this->x, this->z);

    if (this->kinematic_mode) {
        this->computeThrust();
        this->simulateKinematic(dt);
    } else {
        this->computeGravity();
        this->processInput();
        this->updatePosition();
        this->updateSpeedOfSound();
        this->checkStatus();
        this->computeLift();
        this->computeThrust();
        this->computeDrag();
        this->updateForces();
        this->updateAcceleration();
        this->updateVelocity();
    }
    this->updatePlaneStatus();
    // Consommation carburant : facteur MIL/PC x SFC x cran x dt (PhysicsTicks, DATA_MODEL.md §6.2).
    float notch = this->thrust / 10.0f;
    if (this->fuel_kg <= 0.0f) {
        this->fuel_kg = 0.0f;
        this->thrust_force = 0.0f;
    } else {
        float factor = (notch <= 5.0f) ? notch * (51.0f / 256.0f) : notch * (76.0f / 256.0f);
        float burn = (10.0f - 9.0f * this->g_fuel) * this->sfc;
        if (factor > 0.0f) {
            burn *= factor;
        }
        this->fuel_kg -= burn * dt;
        if (this->fuel_kg < 0.0f)
            this->fuel_kg = 0.0f;
    }
    this->fuel = (int)this->fuel_kg;

    if (this->wheels) {
        this->wheel_anim--;
        if (this->wheel_anim == 0) {
            this->wheel_index++;
            if (this->wheel_index > 5)
                this->wheel_index = 5;
            this->wheel_anim = 10;
        }
    } else if (this->wheel_index >= 1) {
        this->wheel_anim--;
        if (this->wheel_anim == 0) {
            this->wheel_index--;
            if (this->wheel_index < 1)
                this->wheel_index = 0;
            this->wheel_anim = 10;
        }
    } else {
        this->wheel_index = 0;
    }

    for (auto sim_obj : this->weaps_object) {
        sim_obj->Simulate(this->tps);
    }
    // Purge des objets d'armement detruits (boucle explicite, pas de lambda).
    size_t writeIndex = 0;
    for (size_t readIndex = 0; readIndex < this->weaps_object.size(); ++readIndex) {
        if (this->weaps_object[readIndex]->alive) {
            this->weaps_object[writeIndex] = this->weaps_object[readIndex];
            ++writeIndex;
        } else {
            this->pilot->mission->onWeaponRemoved(this->weaps_object[readIndex]);
        }
    }
    this->weaps_object.resize(writeIndex);

    this->object->entity->position.x = this->x;
    this->object->entity->position.y = this->y;
    this->object->entity->position.z = this->z;

    const float rise_speed = 1.0f;
    if (this->object->alive == false) {
        Vector3D smoke_position(this->x, this->y, this->z);
        this->smoke_positions.insert(this->smoke_positions.begin(), smoke_position);
        if (this->smoke_anim_counters.size() < this->smoke_set->smoke_textures.size() + 10)
            this->smoke_anim_counters.insert(this->smoke_anim_counters.begin(), 0);
        if (this->smoke_positions.size() > this->smoke_set->smoke_textures.size() + 10) {
            this->smoke_positions.pop_back();
            if (this->smoke_anim_counters.size() > this->smoke_set->smoke_textures.size() + 10)
                this->smoke_anim_counters.pop_back();
        }
        if (this->crached) {
            for (auto &pos : this->smoke_positions) {
                pos.y += rise_speed;
            }
        }
    }

    this->tick_counter++;
    this->azimuthf = this->yaw;
    this->elevationf = this->pitch;
    this->twist = this->roll;
}

float SCJetpPlane::componentGain(const char *first, const char *second) {
    float current = 0.0f;
    float initial = 0.0f;
    for (auto &system : this->object->entity->sysm) {
        for (auto &sub_system : system.second) {
            if (sub_system.first == first || (second != nullptr && sub_system.first == second)) {
                initial += (float) sub_system.second;
                auto health_system = this->system_health.find(system.first);
                if (health_system == this->system_health.end() || health_system->second.find(sub_system.first) == health_system->second.end()) {
                    current += (float) sub_system.second;
                } else {
                    current += (float) health_system->second[sub_system.first];
                }
            }
        }
    }
    if (initial <= 0.0f) {
        return 1.0f;
    }
    return current / initial;
}

void SCJetpPlane::updateMass() {
    float stores_kg = 0.0f;
    for (auto hardpoint : this->weaps_load) {
        if (hardpoint == nullptr || hardpoint->objct == nullptr) {
            continue;
        }
        stores_kg += hardpoint->nb_weap * (float)hardpoint->objct->weight_in_kg;
    }
    this->mass_kg = this->empty_mass_kg + this->fuel_kg + stores_kg;
    this->W = this->mass_kg;
    this->inverse_mass = (this->mass_kg > 0.0f) ? (1.0f / this->mass_kg) : 0.0f;
}

void SCJetpPlane::updateDamageGains() {
    this->g_engine = this->componentGain("ENGINE", nullptr);
    this->g_fuel = this->componentGain("FUEL", nullptr);
    this->g_wing = this->componentGain("LWING", "RWING");
    this->g_elevator = this->componentGain("ELEVATOR", nullptr);
    this->g_rudder = this->componentGain("RUDDER", nullptr);
    this->g_aileron = this->componentGain("AILERON", nullptr);
}

float SCJetpPlane::maxRollRate() {
    float rate = this->max_turn_rate_dps;
    float flow = sqrtf(this->alpha_deg * this->alpha_deg + this->beta_deg * this->beta_deg);
    float onset = this->stall_alpha_deg - 5.0f;
    if (flow > onset) {
        rate /= (flow - onset + 1.0f);
    }
    float V = sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);
    if (V < this->control_speed_ms) {
        rate *= V / this->control_speed_ms;
    }
    return rate;
}
