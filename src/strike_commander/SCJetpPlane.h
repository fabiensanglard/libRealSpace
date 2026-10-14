//
//  SCJetpPlane.h
//  libRealSpace
//
//  Modele de vol reconstruit depuis STRIKE.EXE (reverse engineering), en
//  unites SI, piloté par les donnees des chunks IFF de la FORM DYNM d'un
//  JETP (DYNM=masse, THRS=poussee, STBL=stabilite, JDYN=enveloppe de vol).
//
//  Objet independant : ne remplace ni ne modifie SCPlane (modele generique)
//  ni SCJdynPlane (tentative metrique existante, valeurs devinees). Les
//  trois coexistent pour comparaison directe.
//
//  Reference complete de l'algorithme et de chaque champ :
//  strike_commander_re/analysis/DATA_MODEL.md  (§6.2, citations assembleur)
//  strike_commander_re/analysis/PHYSICS.md     (spec d'implementation)
//
#pragma once

#include "../realspace/RSAirdens.h"

class SCJetpPlane : public SCPlane {
    friend class DebugStrike;

protected:
    // L'etat d'orientation est la matrice ptw, tournee chaque tick par omega*dt (comme
    // WorldObject_IntegrateBodyMotion_3D31D dans l'original) ; pitch/yaw/roll en sont derives.

    // --- Coefficients par avion, charges une fois depuis object->entity ---
    bool entity_loaded{false};

    // Depart en decrochage franc (portance forcee a zero) -- Aero_ComputeLiftAndSideForce loc_481CF.
    // Dans l'ASM il faut EN PLUS difficulte word_70466 > 10 ET option byte_72354 ; ce booleen
    // regroupe ces deux entrees d'etat de jeu (a cabler). La saturation d'incidence + la coupure
    // de force laterale (PHYSICS.md §5.2 cas A) restent actives independamment de ce flag.
    bool realistic_stall{true};

    RSAirdens airdens;   // table DATA\AIRDENS.TBL, chargee dans loadFromEntity()

    float mass_kg{0.0f};                 // masse totale : cellule + carburant + armes
    float empty_mass_kg{0.0f};           // chunk DYNM
    float thrust_max_n{0.0f};            // chunk THRS (poussee pleine post-combustion)
    float thrust_mil_fraction{0.7f};     // chunk THRS, octet 1
    float thrust_ref_alt_fraction{1.0f}; // chunk THRS, octet 2
    float thrust_cutoff_alt_m{23100.0f}; // chunk THRS, octet 3 * 100
    float stability_gain{0.0f};          // chunk STBL
    float parasite_drag{1.0f};           // chunk ATMO

    float fuel_capacity_kg{0.0f};        // JDYN champ 1
    float sfc{0.0f};                     // JDYN champ 2 (consommation specifique)
    float drag_airbrake{0.0f};           // JDYN champ 3
    float drag_gear{0.0f};               // JDYN champ 4
    float ground_moment_1{0.0f};         // JDYN champ 5
    float ground_moment_2{0.0f};         // JDYN champ 6
    float rate_limit_dps{0.0f};          // JDYN champ 7 (limite de variation du taux de roulis)
    float max_turn_rate_dps{0.0f};       // JDYN champ 8 (deg/s)
    float stall_alpha_deg{0.0f};         // JDYN champ 9
    float wing_incidence_deg{0.0f};      // JDYN champ 10
    float flap_lift_increment_deg{0.0f}; // JDYN champ 11
    float pitch_rate_limit_dps{0.0f};    // JDYN champ 15 ("envelope_pitch_limit") : limite de vitesse
                                          // de tangage dediee, distincte de pitch_rate_gain (champ 8,
                                          // reutilise pour roulis/lacet, jamais remis en cause) -
                                          // jamais chargee jusqu'ici, cf. RSEntity.h struct JDYN.
    float control_speed_ms{0.0f};        // JDYN champ 17
    float induced_drag_k{0.0f};          // JDYN champ 18
    float lift_gain{0.0f};               // JDYN champ 19
    float pitch_stick_gain{0.0f};        // JDYN champ 20 ("aileron", jdyn[0x65]) : borne finale de la consigne de tangage
    float yaw_authority{0.0f};           // JDYN champ 21 ("gouverne") : gain palonnier -> consigne de lacet
    float pitch_load_gain{0.0f};         // JDYN champ 22 ("MAX_G", jdyn[0x67]) : gain manche -> demande de charge (tangage)

    float g_engine{1.0f};
    float g_fuel{1.0f};
    float g_wing{1.0f};
    float g_elevator{1.0f};
    float g_rudder{1.0f};
    float g_aileron{1.0f};
    float componentGain(const char *first, const char *second);
    void updateDamageGains();
    void updateMass();

    float fuel_kg{0.0f};                 // carburant courant (precision flottante ; miroir dans SCPlane::fuel)

    // --- Valeurs de travail recalculees a chaque tick ---
    float alpha_deg{0.0f};               // incidence (flux vertical corps)
    float alpha_command_deg{0.0f};
    float beta_deg{0.0f};                // derapage (flux lateral corps)
    float alpha_eff_deg{0.0f};           // incidence effective (calage aile + volets, bornee decrochage)
    float dynamic_pressure{0.0f};        // q = 1/2 . rho(altitude) . V^2
    Vector3D side_vector{0.0f, 0.0f, 0.0f}; // force laterale (derapage), absente de SCPlane

    // --- Etat d'orientation : matrice persistante tournee par increments (cf. updatePosition) ---
    // Mecanisme ASM : Matrix_BuildFullOrientation_575B2 / Matrix_BuildAxisX/Y/Z_56EC3 composent des
    // rotations incrementales EN PLACE sur ptw. pitch/yaw/roll (SCPlane) sont re-derives de ptw
    // chaque tic ; m_seed_* memorise la derniere valeur ecrite pour detecter une ecriture externe.
    bool orientation_seeded{false};
    float m_seed_pitch{0.0f};
    float m_seed_yaw{0.0f};
    float m_seed_roll{0.0f};

    // Idem cote position : `position` (Vector3D) est l'etat maitre de l'integration, mais du code
    // externe (autopilote, teleport, "set altitude") ecrit x/y/z directement. On resynchronise
    // `position` depuis x/y/z quand ils divergent de la derniere valeur ecrite par updatePosition().
    bool position_seeded{false};
    float m_seed_x{0.0f};
    float m_seed_y{0.0f};
    float m_seed_z{0.0f};

    void loadFromEntity();
    float airDensity(float altitude_m) const;
    float throttleNotchToThrustFraction(float notch) const;
    float altitudeLapseFraction(float altitude_m) const;

    void computeLift() override;
    void computeDrag() override;
    void computeGravity() override;
    void computeThrust() override;
    void processInput() override;
    void updateForces() override;
    void updateAcceleration() override;
    void updateVelocity() override;
    void updatePosition() override;
    void updateSpeedOfSound() override;
    void checkStatus() override;
    void updatePlaneStatus() override;
    void syncKinematicVelocity(float dt) override;
    void onPlaneWreck(const PlaneWreckEvent &event) override;

public:
    SCJetpPlane();
    SCJetpPlane(float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE, float s,
                float W, float fuel_weight, float Mthrust, float b, float ie_pi_AR, int MIN_LIFT_SPEED,
                RSArea *area, float x, float y, float z);
    ~SCJetpPlane();
    void Simulate() override;
    float maxRollRate() override;
    Vector3D worldVelocity() override { return this->velocity; }
    float forwardSpeedPerTick() override { return this->tps > 0 ? this->vz / (float) this->tps : this->vz; }
};
