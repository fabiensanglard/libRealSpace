#include "precomp.h"
#define GRAVITY 32.174 // Accélération due à la gravité en m/s^2

int smmrandom(int maxr) {
    static unsigned long randx = 1;
    int n, retval;

    for (n = 1; n < 32 && (1 << n) < maxr; n++)
        ;

    retval = maxr << 1;
    while (retval > maxr) {
        randx = randx * 1103515245 + 12345;
        retval = (randx & 0x7fffffff) >> (31 - n);
    }
    randx = randx * 1103515245 + 12345;
    if (randx & 0x40000000)
        return (retval);
    else
        return (-retval);
}

SCSimulatedObject::SCSimulatedObject() {             
    this->LmaxDEF=10.0f;
    this->LminDEF=-7.0f;

    this->s=1.0f;


    this->Mthrust=0.0f;
    this->b=1.0f;
    this->ie_pi_AR=0.83f;

    this->ro2 = .5f * ro[0];

    this->twist = 0;

    /* elevator rate in degrees/sec	*/
    this->ELEVF = 0;
    /* roll rate (both at 300 mph)	*/
    this->ROLLF = 0;

    this->zdrag = 0.0f;
    this->ydrag = 0.0f;
    /* coefficient of parasitic drag*/
    this->Cdp = .015f;
    /* 1.0 / pi * wing Aspect Ratio	*/
    this->ipi_AR = 1.0f / ((float)M_PI * this->b * this->b / this->s);
    /* 1.0 / pi * AR * efficiency	*/
    this->ie_pi_AR = this->ipi_AR / this->ie_pi_AR;

    this->Lmax = this->LmaxDEF * this->gravity;
    this->Lmin = this->LminDEF * this->gravity;

    this->Cdp *= 2.0;

    this->tilt_factor = 0.17f;
    this->roll_speed = 0;
    this->azimuth_speedf = 0.0f;
    this->elevation_speedf = 0.0f;
    this->elevationf = 0.0f;
    
    this->on_ground = 0;
    this->vx = 0.0f;
    this->vy = 0.0f;
    this->vz = 0.0f;

    this->az = 0.0f;
    this->ay = 0.0f;
    this->ax = 0.0f;

    this->roll_speed = 0;
    this->azimuthf = 0.0f;
    this->elevationf = 0.0f;
    this->elevation_speedf = 0.0f;
    this->azimuth_speedf = 0.0f;
    
    this->ptw.Clear();
    this->incremental.Clear();
}
SCSimulatedObject::~SCSimulatedObject() {

}
void SCSimulatedObject::SimulateWithSameEngineAsPlane(int tps) {
    float temp{0.0f};
    int itemp{0};
    float elevtemp{0.0f};

    this->gravity = G_ACC / tps / tps;
    float inverse_mass = G_ACC / this->weight;
    this->Lmax = this->LmaxDEF * this->gravity;
    this->Lmin = this->LminDEF * this->gravity;

    /* tenths of degrees per tick	*/
    //this->rollers = (this->ROLLF * ((this->control_stick_x + 8) >> 4));
    /* delta */
    itemp = (int)(/*this->rollers * this->vz*/ - this->roll_speed);
    if (itemp != 0) {
        if (itemp >= DELAY || itemp <= -DELAY) {
            itemp /= DELAY;
        } else {
            itemp = itemp > 0 ? 1 : -1;
        }
    }
    if (this->wing_stall > 0) {
        itemp >>= this->wing_stall;
        itemp += smmrandom(this->wing_stall << 3);
    }
    this->roll_speed += itemp;
    //this->elevator = -1.0f * (this->ELEVF * ((this->control_stick_y + 8) >> 4));
    itemp = (int)(/*this->elevator * this->vz*/ + this->vy - this->elevation_speedf);
    elevtemp = /*this->elevator * this->vz*/ + this->vy - this->elevation_speedf;
    if (itemp != 0) {
        if (itemp >= DELAY || itemp <= -DELAY) {
            itemp /= DELAY;
        } else {
            itemp = itemp > 0 ? 1 : -1;
        }
    }
    if (this->wing_stall > 0) {
        itemp >>= this->wing_stall;
        elevtemp = elevtemp / powf(2, this->wing_stall);
        itemp += smmrandom(this->wing_stall * 2);
        elevtemp += smmrandom(this->wing_stall * 2);
    }
    this->elevation_speedf += itemp;
    float aztemp;
    temp = /*this->rudder * this->vz*/ - (2.0f * tps / 20.0f) * this->vx;
    if (this->on_ground) {
        itemp = (int)(16.0f * temp);
        if (itemp < -MAX_TURNRATE || itemp > MAX_TURNRATE) {
            /* clip turn rate	*/
            if (itemp < 0) {
                itemp = -MAX_TURNRATE;
            } else {
                itemp = MAX_TURNRATE;
            }
            /* decrease with velocity */
            if (fabs(this->vz) > 10.0f / tps) {
                /* skid effect */
                temp = 0.4f * tps * (/*this->rudder * this->vz*/ - .75f);
                if (temp < -MAX_TURNRATE || temp > MAX_TURNRATE) {
                    temp = (float)itemp;
                }
                itemp -= (int)temp;
            }
        }
        temp = (float)itemp;
    } else {
        /* itemp is desired azimuth speed	*/
        itemp = (int)temp;
    }

    aztemp = temp;
    /* itemp is now desired-actual		*/
    itemp -= (int)this->azimuth_speedf;
    aztemp -= this->azimuth_speedf;
    if (itemp != 0) {
        if (itemp >= DELAY || itemp <= -DELAY) {
            itemp /= DELAY;
        } else {
            itemp = itemp > 0 ? 1 : -1;
        }
    }
    this->azimuth_speedf += itemp;
    if (this->on_ground) {
        /* dont allow negative pitch unless positive elevation	*/
        if (this->elevation_speedf < 0) {
            if (this->elevationf <= 0) {
                this->elevation_speedf = 0;
            }
        } else {
            /* mimic gravitational torque	*/
            elevtemp = -(this->vz * tps + 1) / 4.0f;
            if (elevtemp < 0 && this->elevationf <= 0) {
                elevtemp = 0.0f;
            }
            if (this->elevation_speedf > elevtemp) {
                this->elevation_speedf = elevtemp;
            }
        }
        this->roll_speed = 0;
    }
    
    /* compute polar angle from (vx, vy, vz) vector */
    float v{0.0f};
    v = sqrtf(this->vx * this->vx + this->vy * this->vy + this->vz * this->vz);
    float theta{0.0f};
    float phi{0.0f};

    if (v != 0.0) {
        theta = acosf(-this->vy / v);
        phi = atan2f(-this->vz, -this->vx);
    } else {
        theta = 0.0;
        phi = 0.0;
    }
    //this->elevationf = ((1-theta) * 180.0f / (float)M_PI) * 10;
    //this->azimuthf = ((1-phi) * 180.0f / (float)M_PI) * 10;
    /*if (this->azimuthf < 0) {
        this->azimuthf += 3600;
    } else if (this->azimuthf >= 3600) {
        this->azimuthf -= 3600;
    }*/
    if (this->tick_counter % 100 == 0) {
        this->ptw.Identity();
        this->ptw.translateM(this->x, this->y, this->z);
        this->ptw.rotateM(tenthOfDegreeToRad(this->azimuthf), 0, 1, 0);
        this->ptw.rotateM(tenthOfDegreeToRad(this->elevationf), 1, 0, 0);
        this->ptw.rotateM(tenthOfDegreeToRad(this->twist), 0, 0, 1);
    }
    this->ptw.translateM(this->vx, this->vy, this->vz);
    if (round(this->azimuth_speedf) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad(roundf(this->azimuth_speedf)), 0, 1, 0);
    if (round(this->elevation_speedf) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad(roundf(this->elevation_speedf)), 1, 0, 0);
    if (round(this->roll_speed) != 0)
        this->ptw.rotateM(tenthOfDegreeToRad((float)this->roll_speed), 0, 0, 1);
    temp = 0.0f;
    
    this->elevationf = (-asinf(this->ptw.v[2][1]) * 180.0f / (float)M_PI) * 10;

    float ascos = 0.0f;

    temp = cosf(tenthOfDegreeToRad(this->elevationf));

    if (temp != 0.0) {

        float sincosas = this->ptw.v[2][0] / temp;

        if (sincosas > 1) {
            sincosas = 1.0f;
        } else if (sincosas < -1) {
            sincosas = -1;
        }
        this->azimuthf = asinf(sincosas) / (float)M_PI * 1800.0f;
        if (this->ptw.v[2][2] < 0.0) {
            this->azimuthf = 1800 - this->azimuthf;
        }
        if (this->azimuthf < 0) {
            this->azimuthf += 3600;
        }

        this->twist = (asinf(this->ptw.v[0][1] / temp) / (float)M_PI) * 1800.0f;
        if (this->ptw.v[1][1] < 0.0) {
            this->twist = 1800.0f - this->twist;
        }
        if (this->twist < 0) {
            this->twist += 3600.0f;
        }
    }
    /* save last position	*/
    this->last_px = this->x;
    this->last_py = this->y;
    this->last_pz = this->z;
    this->x = this->ptw.v[3][0];
    this->y = this->ptw.v[3][1];
    this->z = this->ptw.v[3][2];

    /****************************************************************/
    /*	perform incremental rotations on velocities                 */
    /****************************************************************/

    this->incremental.Identity();
    if (this->roll_speed)
        this->incremental.rotateM(tenthOfDegreeToRad((float)-this->roll_speed), 0, 0, 1);
    if (this->elevation_speedf)
        this->incremental.rotateM(tenthOfDegreeToRad(-this->elevation_speedf), 1, 0, 0);
    if (this->azimuth_speedf)
        this->incremental.rotateM(tenthOfDegreeToRad(-this->azimuth_speedf), 0, 1, 0);
    this->incremental.translateM(this->vx, this->vy, this->vz);

    this->vx = this->incremental.v[3][0];
    this->vy = this->incremental.v[3][1];
    this->vz = this->incremental.v[3][2];
    float val;
    /* compute new accelerations, lift: only if vz is negative	*/
    val = (this->vz >= 0.0);
    if (!val) {
        this->ae = this->vy / this->vz + this->tilt_factor;
        this->Cl = this->uCl = this->ae / (.17f + this->kl * this->ipi_AR);
        /* check for positive stall	*/
        if (this->Cl > this->max_cl) {
            this->Cl = 3.0f * this->max_cl - 2.0f * this->Cl;
            this->wing_stall = 1;
            if (this->Cl < 0.0f) {
                this->wing_stall += 1 - (int)(this->Cl / this->max_cl);
                this->Cl = 0.0f;
            }
            if (this->uCl > 5.0f) {
                this->uCl = 5.0f;
            }
        } else if (this->Cl < this->min_cl) {
            /* check for negative stall	*/
            this->Cl = 3.0f * this->min_cl - 2.0f * this->Cl;
            this->wing_stall = 1;
            if (this->Cl > 0.0f) {
                this->wing_stall += 1 - (int)(this->Cl / this->min_cl);
                this->Cl = 0.0f;
            }
            if (this->uCl < -5.0f) {
                this->uCl = -5.0f;
            }
        } else {
            this->wing_stall = FALSE;
        }
    } else {
        this->Cl = this->uCl = 0.0f;
        this->wing_stall = this->on_ground ? 0 : (int)this->vz;
        this->ae = 0.0f;
    }
    if (this->wing_stall > 64) {
        this->wing_stall = 64;
    }
    if ((this->tick_counter & 1) == 0) {
        /* only do on even ticks	*/
        /* compute speed of sound	*/
        if (this->y <= 36000.0f) {
            this->sos = -1116.0f / tps + (1116.0f - 968.0f) / tps / 36000.0f * this->y;
        } else {
            this->sos = -968.0f / tps;
        }
        itemp = ((int)this->y) >> 10;
        if (itemp > 74) {
            itemp = 74;
        } else if (itemp < 0) {
            itemp = 0;
        }
        this->ro2 = .5f * ro[itemp];
        if (this->Cl < .2) {
            this->mcc = .7166666f + .1666667f * this->Cl;
        } else {
            this->mcc = .7833333f - .1666667f * this->Cl;
        }
        /* and current mach number */
        this->mach = this->vz / this->sos;
        this->mratio = this->mach / this->mcc;
        if (this->mratio < 1.034f) {
            this->Cdc = 0.0f;
        } else {
            this->Cdc = .082f * this->mratio - 0.084788f;
            if (this->Cdc > .03f)
                this->Cdc = .03f;
        }
    }

    /* assume V approx = vz	*/
    this->qs = this->ro2 * this->vz * this->s;

    this->lift = this->Cl * this->qs;

    this->ay = this->vz * this->lift;
    this->az = -this->vy * this->lift;
    /* save for wing loading meter	*/
    this->lift = this->ay * inverse_mass;

    /* engine thrust		*/
    this->az = this->az - (.01f / tps / tps * 100 * this->Mthrust);

    /* drag - needs to be broken into y/z components		*/
    this->Cd = this->Cdp + this->kl * this->uCl * this->uCl * this->ie_pi_AR + this->Cdc;
    this->zdrag = this->Cd * this->qs;
    this->ydrag = this->vy * this->zdrag;
    this->zdrag = this->vz * this->zdrag;
    /* if vz is positive	*/
    if (val) {
        this->ay -= this->ydrag;
        this->az -= this->zdrag;
    } else {
        this->ay += this->ydrag;
        this->az += this->zdrag;
    }

    /* now convert forces to accelerations (A=F/M)	*/
    this->ax = 0.0f;
    this->ay *= inverse_mass;
    this->az *= inverse_mass;

    /* add in gravity which is an acceleration	*/
    /**/
    this->ax -= this->ptw.v[0][1] * this->gravity;
    this->ay -= this->ptw.v[1][1] * this->gravity;
    this->az -= this->ptw.v[2][1] * this->gravity;
    /**/
    this->vx += this->ax;
    this->vz += this->az;
    this->vy += this->ay;

    this->tick_counter++;
}
void SCSimulatedObject::Render() {
    glPushMatrix();
    Matrix rotation;
    rotation.Clear();
    rotation.Identity();
    rotation.translateM(this->x * COORD_SCALE, this->y * COORD_SCALE, this->z * COORD_SCALE);
    rotation.rotateM(((this->azimuthf + 900) / 10.0f) * ((float)M_PI / 180.0f), 0.0f, 1.0f, 0.0f);
    rotation.rotateM((this->elevationf / 10.0f) * ((float)M_PI / 180.0f), 0.0f, 0.0f, 1.0f);
    rotation.rotateM(-(this->twist / 10.0f) * ((float)M_PI / 180.0f), 1.0f, 0.0f, 0.0f);

    glMultMatrixf((float *)rotation.v);
    
    Renderer.DrawModel(this->obj, LOD_LEVEL_MAX);
    glPopMatrix();
}
void SCSimulatedObject::GetPosition(Vector3D *position) {
    position->x = this->x * COORD_SCALE;
    position->y = this->y * COORD_SCALE;
    position->z = this->z * COORD_SCALE;
}
// Fonction pour convertir les coordonnées polaires en coordonnées cartésiennes
Vector3D polarToCartesian(float speed, float phi, float theta) {
    Vector3D v;
    v.x = speed * sinf(theta) * cosf(phi);
    v.y = speed * sinf(theta) * sinf(phi);
    v.z = speed * cosf(theta);
    return v;
}

// Fonction pour convertir les coordonnées cartésiennes en coordonnées polaires
void cartesianToPolar(Vector3D v, float *phi, float *theta) {
    *theta = acosf(v.y / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z));
    *phi = atan2f(v.z, v.x);
}

// Fonction pour calculer la nouvelle vitesse en tenant compte de la gravité et de la résistance de l'air
Vector3D calculateNewVelocity(Vector3D velocity, float mass, float dragCoefficient, float deltaTime) {
    Vector3D newVelocity;
    float speed = sqrtf((velocity.x*velocity.x) + (velocity.y*velocity.y) + (velocity.z*velocity.z));

    float dragForceX = -dragCoefficient * velocity.x * speed/mass;
    float dragForceY = -dragCoefficient * velocity.y * speed/mass;
    float dragForceZ = -dragCoefficient * velocity.z * speed/mass;

    newVelocity.x = velocity.x + dragForceX * deltaTime;
    newVelocity.y = velocity.y + (-1.0f * GRAVITY + dragForceY) * deltaTime;
    newVelocity.z = velocity.z + dragForceZ * deltaTime;

    return newVelocity;
}
// Fonction pour calculer la magnitude de la vitesse
float calculateSpeed(Vector3D velocity) {
    return sqrtf(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
}
void SCSimulatedObject::Simulate(int tps) {
    float deltaTime = 1.0f / (float) tps; // Intervalle de temps entre chaque calcul

    Vector3D planeVelocity = {
        this->vx,
        this->vy,
        this->vz
    };
    Vector3D newVelocity = calculateNewVelocity(planeVelocity, this->weight, 0.5f, deltaTime);
    
    this->x = this->x+newVelocity.x;
    this->y = this->y+newVelocity.y;
    this->z = this->z+newVelocity.z;

    this->vx = newVelocity.x;
    this->vy = newVelocity.y;
    this->vz = newVelocity.z;
    float phi, theta;
    cartesianToPolar(newVelocity, &phi, &theta);
    speed = calculateSpeed(newVelocity);
    this->azimuthf = ((M_PI / 2.0f + phi) * 180.0f / (float) M_PI * 10.0f);
    this->elevationf = (M_PI / 2.0f - theta) * 180.0f / (float) M_PI * 10.0f;
    if (this->y<0) {
        this->alive = false;
    }
}