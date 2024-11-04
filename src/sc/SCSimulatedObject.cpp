#include "precomp.h"


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

    this->s=10.0f;


    this->Mthrust=0.0f;
    this->b=32.0f;
    this->ie_pi_AR=.93f;

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
    
    this->on_ground = 1;
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
void SCSimulatedObject::Simulate(int tps) {
    float temp{0.0f};
    int itemp{0};
    float elevtemp{0.0f};

    this->gravity = G_ACC / tps / tps;
    float inverse_mass = G_ACC / this->weight;
    this->Lmax = this->LmaxDEF * this->gravity;
    this->Lmin = this->LminDEF * this->gravity;

    
    
    /* delta */
    itemp = (int)(this->roll_speed);
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
    
    itemp = (int)(this->vy - this->elevation_speedf);
    elevtemp = this->vy - this->elevation_speedf;
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
    temp = (2.0f * tps / 20.0f) * this->vx;
    
    /* itemp is desired azimuth speed	*/
    itemp = (int)temp;
    

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

    /****************************************************************
    /*	perform incremental rotations on velocities
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
        /* and current mach number	*/
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