//
//  SCPlane.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 31/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include "precomp.h"



/**
 * \brief Computes sine and cosine of angle a, given in 10th of degree.
 *
 * \param a angle in 10th of degree.
 * \param b pointer to float where the sine of a will be stored (if not null).
 * \param c pointer to float where the cosine of a will be stored (if not null).
 */
void gl_sincos(float a, float *b, float *c) {
    if (b != NULL) {
        *b = sinf(tenthOfDegreeToRad(a));
    }
    if (c != NULL) {
        *c = cosf(tenthOfDegreeToRad(a));
    }
}
/**
 * Returns a random number between 0 and maxr (inclusive) using a
 * simple linear congruential generator. The seed is initialized to
 * 1 at startup and is advanced every time the function is called.
 *
 * @param maxr The maximum value that can be returned.
 *
 * @returns A random number between 0 and maxr (inclusive). If maxr
 *          is odd, the returned value can be negative.
 */
int mrandom(int maxr) {
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

/**
 * SCPlane constructor. Initializes all members to 0.
 *
 * SCPlane is a structure that represents a plane in the game.
 *
 * @see SCPlane
 */
SCPlane::SCPlane() {
    this->planeid = 0;
    this->version = 0;
    this->cmd = 0;
    this->type = 0;
    this->alive = 0;
    this->myname[0] = '\0';
    this->status = 0;
    this->won = 0;
    this->lost = 0;
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
    this->twist = 0;
    this->roll_speed = 0;
    this->azimuthf = 0.0f;
    this->elevationf = 0.0f;
    this->elevation_speedf = 0.0f;
    this->azimuth_speedf = 0.0f;
    this->airspeed = 0;
    this->thrust = 0;
    this->mtype = 0;
    this->rollers = 0.0f;
    this->rudder = 0.0f;
    this->elevator = 0.0f;
    this->object = nullptr;
}
/**
 * Constructor for SCPlane.
 *
 * @param LmaxDEF the maximum lift coefficient.
 * @param LminDEF the minimum lift coefficient.
 * @param Fmax the maximum flap deflection.
 * @param Smax the maximum spoiler deflection.
 * @param ELEVF_CSTE the elevator rate in degrees per second.
 * @param ROLLFF_CSTE the roll rate in degrees per second.
 * @param s the wing span.
 * @param W the plane weight.
 * @param fuel_weight the fuel weight.
 * @param Mthrust the maximum thrust.
 * @param b the wing aspect ratio.
 * @param ie_pi_AR the inverse of the aspect ratio times pi.
 * @param MIN_LIFT_SPEED the minimum lift speed.
 * @param pilot_y the pilot y position.
 * @param pilot_z the pilot z position.
 * @param area the terrain area.
 * @param x the initial x position.
 * @param y the initial y position.
 * @param z the initial z position.
 */
SCPlane::SCPlane(float LmaxDEF, float LminDEF, float Fmax, float Smax, float ELEVF_CSTE, float ROLLFF_CSTE, float s,
                 float W, float fuel_weight, float Mthrust, float b, float ie_pi_AR, int MIN_LIFT_SPEED, float pilot_y,
                 float pilot_z, RSArea *area, float x, float y, float z) {
    this->weaps_load.reserve(9);
    this->weaps_load.resize(9);
    this->planeid = 0;
    this->version = 0;
    this->cmd = 0;
    this->type = 0;
    this->alive = 0;
    this->myname[0] = '\0';
    this->status = 0;
    this->won = 0;
    this->lost = 0;
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
    this->twist = 0;
    this->roll_speed = 0;
    this->azimuthf = 0.0f;
    this->elevationf = 0.0f;
    this->elevation_speedf = 0.0f;
    this->azimuth_speedf = 0.0f;
    this->airspeed = 0;
    this->thrust = 0;
    this->mtype = 0;
    this->rollers = 0.0f;
    this->rudder = 0.0f;
    this->elevator = 0.0f;
    this->object = nullptr;

    this->LmaxDEF = LmaxDEF;
    this->LminDEF = LminDEF;
    this->Fmax = Fmax;
    this->Smax = Smax;
    this->ELEVF_CSTE = ELEVF_CSTE;
    this->ROLLFF_CSTE = ROLLFF_CSTE;
    this->obj = obj;
    this->s = s;
    this->W = W;
    this->fuel_weight = fuel_weight;
    this->Mthrust = Mthrust;
    this->b = b;
    this->ie_pi_AR = ie_pi_AR;
    this->MAX_RK = MAX_RK;
    this->MAX_SW = MAX_SW;
    this->MIN_LIFT_SPEED = MIN_LIFT_SPEED;
    this->pilot_y = pilot_y;
    this->pilot_z = pilot_z;
    this->object = nullptr;
    this->area = area;
    this->tps = 30;
    this->last_time = SDL_GetTicks();
    this->tick_counter = 0;
    this->last_tick = 0;
    this->x = x / COORD_SCALE;
    this->y = y / COORD_SCALE;
    this->z = z / COORD_SCALE;
    this->ro2 = .5f * ro[0];
    Init();
}
SCPlane::~SCPlane() {}
/**
 * Initializes the SCPlane object by setting the default values for all its properties.
 */
void SCPlane::Init() {

    this->twist = 0;

    this->status = 580000;
    this->alive = this->tps << 2;
    this->flaps = 0;
    this->spoilers = 0;
    this->rollers = 0;
    this->rudder = 0;
    this->elevator = 0;
    /* elevator rate in degrees/sec	*/
    this->ELEVF = this->ELEVF_CSTE * 10.0f / (20.0f * 400.0f);
    /* roll rate (both at 300 mph)	*/
    this->ROLLF = this->ROLLFF_CSTE * 10.0f / (30.0f * 400.0f);

    this->zdrag = 0.0f;
    this->ydrag = 0.0f;
    /* coefficient of parasitic drag*/
    this->Cdp = .015f;
    /* 1.0 / pi * wing Aspect Ratio	*/
    this->ipi_AR = 1.0f / ((float)M_PI * this->b * this->b / this->s);
    /* 1.0 / pi * AR * efficiency	*/
    this->ie_pi_AR = this->ipi_AR / this->ie_pi_AR;
    this->gravity = G_ACC / this->tps / this->tps;
    this->fps_knots = this->tps * (3600.0f / 6082.0f);
    this->Lmax = this->LmaxDEF * this->gravity;
    this->Lmin = this->LminDEF * this->gravity;
    this->wheels = 1;
    this->Cdp *= 2.0;
    this->fuel = 100 << 7;
    this->gefy = .7f * this->b;
    this->thrust = 0;
    this->tilt_factor = 0.17f;
    this->roll_speed = 0;
    this->azimuth_speedf = 0.0f;
    this->elevation_speedf = 0.0f;
    this->elevationf = 0.0f;
    this->inverse_mass = G_ACC / (this->W + this->fuel / 12800.0f * this->fuel_weight);
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
    this->vx_add = 0.0f;
    this->vy_add = 0.0f;
    this->vz_add = 0.0f;
    this->control_stick_x = 0;
    this->control_stick_y = 0;
    this->ptw.Clear();
    this->incremental.Clear();
}
/**
 * Simulate the plane's motion.
 *
 * This is the main loop of the simulator. It is responsible for updating the
 * plane's state based on user input and physics.
 *
 * The simulation is divided into two parts: the first part updates the plane's
 * orientation and position based on the user's control inputs, and the second
 * part updates the plane's velocity and acceleration based on the physics of
 * flight.
 *
 * The simulation is done in discrete time steps, with the time step being
 * determined by the TPS (ticks per second) value. The simulation is done in
 * two parts so that the user can see the plane's response to control inputs
 * immediately, rather than having to wait for the next time step.
 *
 * The simulation is also responsible for updating the plane's state variables
 * such as its position, velocity, and acceleration. These variables are used
 * by the rendering code to draw the plane on the screen.
 */
void SCPlane::Simulate() {
    int itemp;
    float temp;
    float elevtemp = 0.0f;

    uint32_t current_time = SDL_GetTicks();
    uint32_t elapsed_time = (current_time - this->last_time) / 1000;
    int newtps = 0;
    if (elapsed_time > 1) {
        uint32_t ticks = this->tick_counter - this->last_tick;
        newtps = ticks / elapsed_time;
        this->last_time = current_time;
        this->last_tick = this->tick_counter;
        if (newtps > this->tps / 2) {    
            this->tps = newtps;
        }
    }

    this->gravity = G_ACC / this->tps / this->tps;
    this->fps_knots = this->tps * (3600.0f / 6082.0f);
    this->Lmax = this->LmaxDEF * this->gravity;
    this->Lmin = this->LminDEF * this->gravity;

    this->max_cl = 1.5f + this->flaps / 62.5f;
    this->min_cl = this->flaps / 62.5f - 1.5f;
    this->tilt_factor = .005f * this->flaps + .017f;

    this->Spdf = .0025f * this->spoilers;
    this->Splf = 1.0f - .005f * this->spoilers;

    float groundlevel = this->area->getY(this->x * COORD_SCALE, this->z * COORD_SCALE) / COORD_SCALE;

    if (this->status > MEXPLODE) {
        /* tenths of degrees per tick	*/
        this->rollers = (this->ROLLF * ((this->control_stick_x + 8) >> 4));
        /* delta */
        itemp = (int)(this->rollers * this->vz - this->roll_speed);
        if (itemp != 0) {
            if (itemp >= DELAY || itemp <= -DELAY) {
                itemp /= DELAY;
            } else {
                itemp = itemp > 0 ? 1 : -1;
            }
        }
        if (this->wing_stall > 0) {
            itemp >>= this->wing_stall;
            itemp += mrandom(this->wing_stall << 3);
        }
        this->roll_speed += itemp;
        this->elevator = -1.0f * (this->ELEVF * ((this->control_stick_y + 8) >> 4));
        itemp = (int)(this->elevator * this->vz + this->vy - this->elevation_speedf);
        elevtemp = this->elevator * this->vz + this->vy - this->elevation_speedf;
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
            itemp += mrandom(this->wing_stall * 2);
            elevtemp += mrandom(this->wing_stall * 2);
        }
        this->elevation_speedf += itemp;
        float aztemp;
        temp = this->rudder * this->vz - (2.0f * this->tps / 20.0f) * this->vx;
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
                if (fabs(this->vz) > 10.0f / this->tps) {
                    /* skid effect */
                    temp = 0.4f * this->tps * (this->rudder * this->vz - .75f);
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
                elevtemp = -(this->vz * this->tps + this->MIN_LIFT_SPEED) / 4.0f;
                if (elevtemp < 0 && this->elevationf <= 0) {
                    elevtemp = 0.0f;
                }
                if (this->elevation_speedf > elevtemp) {
                    this->elevation_speedf = elevtemp;
                }
            }
            this->roll_speed = 0;
        }

        /****************************************************************
        /*	concat incremental rotations and get new angles back
        /****************************************************************/
        if (this->tick_counter % 100 == 0) {
            // rebuild old ptw
            // to keep it normalized

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

        /* analyze new ptw	*/
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
                /* if heading into z	*/

                this->azimuthf = 1800 - this->azimuthf;
            }
            if (this->azimuthf < 0) {
                /* then adjust		*/
                this->azimuthf += 3600;
            }

            this->twist = (asinf(this->ptw.v[0][1] / temp) / (float)M_PI) * 1800.0f;
            if (this->ptw.v[1][1] < 0.0) {
                /* if upside down	*/
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

        /****************************************************************/
        /*	check altitude for too high, and landing/takeoff            */
        /****************************************************************/
        /* flame out		*/
        if (this->y > 50000.0)
            this->thrust = 0;
        else if (this->y > groundlevel + 4.0) {
            /* not on ground	*/

            if (this->on_ground) {
                this->Cdp /= 3.0;
                this->min_throttle = 0;
            }
            this->on_ground = FALSE;
        } else if (this->y < groundlevel) {
            /* check for on the ground */
            if (this->nocrash == 0) {
                if (this->isOnRunWay())
                    /* and not on ground before */
                    if (!this->on_ground) {
                        int rating;
                        /* increase drag	*/
                        this->Cdp *= 3.0;
                        /* allow reverse engines*/
                        this->min_throttle = -this->max_throttle;
                        rating = report_card(-this->climbspeed, this->twist, (int)(this->vx * this->tps),
                                             (int)(-this->vz * this->fps_knots), this->wheels);
                        /* oops - you crashed	*/
                        if (rating == -1) {
                            /* set to exploding	*/
                            this->status = MEXPLODE;
                            /* increment count	*/
                            this->lost++;
                        } else {
                            this->fuel += rating << 7;
                            if (this->fuel > (100 << 7))
                                this->fuel = 100 << 7;
                            this->max_throttle = 100;
                        }
                    } else {
                        this->status = MEXPLODE;
                    }
            }
            this->ptw.v[3][1] = this->y = groundlevel;
            this->on_ground = TRUE;
            if (this->status > MEXPLODE) {
                /* kill negative elevation */
                if (this->elevationf < 0) {
                    this->elevationf = 0;
                }
                /* kill any twist	*/
                if (this->twist != 0) {
                    this->twist = 0;
                }
            }
        }
    } /* end not crashing	*/

    /****************************************************************
    /*	compute new velocities, accelerations
    /****************************************************************/

    /* out of gas	*/
    if (this->fuel <= 0) {
        this->thrust = 0;
        this->max_throttle = 0;
        this->min_throttle = 0;
    }

    if (this->status > MEXPLODE) {
        if (this->y > this->gefy) {
            // ground effect factor
            this->kl = 1.0f;
        } else {
            this->kl = (this->y / this->b);
            if (this->kl > .225f) {
                this->kl = .484f * this->kl + .661f;
            } else {
                this->kl = 2.533f * this->kl + .20f;
            }
        }

        /* compute new accelerations, lift: only if vz is negative	*/
        this->val = (this->vz >= 0.0);
        if (!this->val) {
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
                this->sos = -1116.0f / this->tps + (1116.0f - 968.0f) / this->tps / 36000.0f * this->y;
            } else {
                this->sos = -968.0f / this->tps;
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
            if (this->spoilers > 0.0f) {
                this->Cdc += this->Spdf;
            }
        }

        /* assume V approx = vz	*/
        this->qs = this->ro2 * this->vz * this->s;

        this->lift = this->Cl * this->qs;
        this->g_limit = FALSE;
        if (this->spoilers > 0) {
            this->lift *= this->Splf;
        }
    relift:
        this->ay = this->vz * this->lift;
        this->az = -this->vy * this->lift;
        /* save for wing loading meter	*/
        this->lift = this->ay * this->inverse_mass;
        if (this->lift > this->Lmax) {
            this->lift = .99f * this->Lmax / this->inverse_mass / this->vz;
            this->g_limit = TRUE;
            goto relift;
        } else if (this->lift < this->Lmin) {
            this->lift = .99f * this->Lmin / this->inverse_mass / this->vz;
            this->g_limit = TRUE;
            goto relift;
        }

        /* engine thrust		*/
        this->az = this->az - (.01f / this->tps / this->tps * this->thrust * this->Mthrust);

        /* drag - needs to be broken into y/z components		*/
        this->Cd = this->Cdp + this->kl * this->uCl * this->uCl * this->ie_pi_AR + this->Cdc;
        this->zdrag = this->Cd * this->qs;
        this->ydrag = this->vy * this->zdrag;
        this->zdrag = this->vz * this->zdrag;
        /* if vz is positive	*/
        if (this->val) {
            this->ay -= this->ydrag;
            this->az -= this->zdrag;
        } else {
            this->ay += this->ydrag;
            this->az += this->zdrag;
        }

        /* now convert forces to accelerations (A=F/M)	*/
        this->ax = 0.0f;
        this->ay *= this->inverse_mass;
        this->az *= this->inverse_mass;

        /* add in gravity which is an acceleration	*/
        /**/
        this->ax -= this->ptw.v[0][1] * this->gravity;
        this->ay -= this->ptw.v[1][1] * this->gravity;
        this->az -= this->ptw.v[2][1] * this->gravity;
        /**/
        this->vx += this->ax;
        this->vz += this->az;
        if (this->on_ground && this->status > MEXPLODE) {
            temp = 0.0f;
            float mcos;
            this->vx = 0.0;
            gl_sincos(this->elevationf, &temp, &mcos);
            temp = this->vz * temp / mcos;
            if (this->vy + this->ay < temp) {
                this->ay = temp - this->vy;
            }
        }
        this->vy += this->ay;

        this->airspeed = -(int)(this->fps_knots * this->vz);
        this->climbspeed = (short)(this->tps * (this->y - this->last_py));

        this->vx += this->vx_add;
        this->vy += this->vy_add;
        this->vz += this->vz_add;
        if (this->thrust > 0) {
            itemp = this->thrust;
        } else {
            itemp = -this->thrust;
        }
        if (this->tick_counter % (100 * TPS) == 1) {
            this->fuel_rate = fuel_consump(this->Mthrust, this->W);
            this->fuel -= (int)(itemp * this->fuel_rate);
            this->inverse_mass = G_ACC / (this->W + this->fuel / 12800.0f * this->fuel_weight);
        }
    }
    this->tick_counter++;
}

/**
 * IN_BOX: Check if the plane is inside a box.
 * 
 * @param llx Lower left X of the box.
 * @param urx Upper right X of the box.
 * @param llz Lower left Z of the box.
 * @param urz Upper right Z of the box.
 * @return 1 if the plane is inside, 0 otherwise.
 */
int SCPlane::IN_BOX(int llx, int urx, int llz, int urz) {
    return (llx <= this->x && this->x <= urx && llz <= this->z && this->z <= urz);
}
/**
 * Check if the plane is currently on a runway.
 *
 * @return 1 if the plane is on a runway, 0 otherwise.
 */
int SCPlane::isOnRunWay() {

    for (int i = 0; i < area->objectOverlay.size(); i++) {

        if (IN_BOX(area->objectOverlay[i].lx, area->objectOverlay[i].hx, area->objectOverlay[i].ly,
                   area->objectOverlay[i].hy)) {

            return 1;
        }
    }
    return 0;
}
float SCPlane::fuel_consump(float f, float b) { return 0.3f * f / b; }
/**
 * report_card: Evaluate the landing quality of the plane.
 *
 * @param descent_rate Vertical speed of the plane at landing.
 * @param roll_angle Angle of roll at landing.
 * @param velocity_x Horizontal speed of the plane at landing.
 * @param velocity_z Vertical speed of the plane at landing.
 * @param wheels_down Is the landing gear down?
 *
 * @return A score indicating the quality of the landing:
 *         1: Good
 *         0: Fair
 *        -1: Bad
 */
int SCPlane::report_card(int descent_rate, float roll_angle, int velocity_x, int velocity_z, int wheels_down) {
    int on_runway = isOnRunWay();
    int rating = 1;

    roll_angle /= 10;
    if (roll_angle > 180)
        roll_angle -= 360;

    if (!wheels_down)
        rating = 0;
    if (descent_rate > 100)
        rating = 0;
    if (roll_angle < 0)
        roll_angle = -roll_angle;
    if (roll_angle > 10)
        rating = 0;
    if (!on_runway)
        rating = 0;
    else if (velocity_x > 10 || velocity_x < -10)
        rating = 0;

    if (roll_angle > 20 || descent_rate > 20 || velocity_x > 20 || velocity_x < -20)
        rating = -1;

    return rating;
}

void SCPlane::SetThrottle(int throttle) { this->thrust = throttle; }
int SCPlane::GetThrottle() { return this->thrust; }
void SCPlane::SetFlaps() {
    if (this->flaps == this->Fmax) {
        this->flaps = 0;
    } else {
        this->flaps = (int) this->Fmax;
    }
}
void SCPlane::SetSpoilers() {
    if (this->spoilers == this->Smax) {
        this->spoilers = 0;
    } else {
        this->spoilers = (int) this->Smax;
    }
}
void SCPlane::SetWheel() { this->wheels = !this->wheels; }
short SCPlane::GetWheel() { return this->wheels; }
int SCPlane::GetFlaps() { return this->flaps; }
int SCPlane::GetSpoilers() { return this->spoilers; }
void SCPlane::SetControlStick(int x, int y) {
    this->control_stick_x = x;
    this->control_stick_y = y;
}
void SCPlane::getPosition(Point3D *position) {
    position->x = this->x * COORD_SCALE;
    position->y = this->y * COORD_SCALE;
    position->z = this->z * COORD_SCALE;
}
/**
 * Renders the plane object in 3D space. The plane is positioned using the
 * plane's x, y and z coordinates, and rotated using the plane's azimuth and
 * elevation angles. The plane is drawn using the highest level of detail.
 */
void SCPlane::Render() {
    if (this->object != nullptr) {
        glPushMatrix();
        Matrix rotation;
        rotation.Clear();
        rotation.Identity();
        rotation.translateM(this->x * COORD_SCALE, this->y * COORD_SCALE, this->z * COORD_SCALE);
        rotation.rotateM(((this->azimuthf + 900) / 10.0f) * ((float)M_PI / 180.0f), 0.0f, 1.0f, 0.0f);
        rotation.rotateM((this->elevationf / 10.0f) * ((float)M_PI / 180.0f), 0.0f, 0.0f, 1.0f);
        rotation.rotateM(-(this->twist / 10.0f) * ((float)M_PI / 180.0f), 1.0f, 0.0f, 0.0f);

        glMultMatrixf((float *)rotation.v);
        
        Renderer.DrawModel(this->object->entity, LOD_LEVEL_MAX);
        if (this->wheels) {
            Renderer.DrawModel(this->object->entity->chld[wheel_index]->objct, LOD_LEVEL_MAX);
            this->wheel_anim --;
            if (this->wheel_anim == 0) {
                this->wheel_index ++;
                if (this->wheel_index > 5) {
                    this->wheel_index = 5;
                }
                this->wheel_anim = 10;
            }
        } else {
            if (this->wheel_index>=1) {
                Renderer.DrawModel(this->object->entity->chld[wheel_index]->objct, LOD_LEVEL_MAX);
                this->wheel_anim --;
                if (this->wheel_anim == 0) {
                    this->wheel_index --;
                    if (this->wheel_index < 1) {
                        this->wheel_index = 0;
                    }
                    this->wheel_anim = 10;
                }
            } else {
                this->wheel_index = 0;
            }
            
        }
        if (this->thrust > 50) {
            glPushMatrix();
            Vector3D pos = {
                this->object->entity->chld[0]->x,
                this->object->entity->chld[0]->y,
                this->object->entity->chld[0]->z
            };
            glTranslatef(pos.y/250 , pos.z /250 , pos.x /250);
            glScalef(1+this->thrust/100.0f,1,1);
            Renderer.DrawModel(this->object->entity->chld[0]->objct, LOD_LEVEL_MAX);
            glPopMatrix();
        }
        for (auto weaps:this->weaps_load) {
            float decy=0.5f;
            if (weaps->hpts_type == 0) {
                continue;
            }
            if (weaps->hpts_type == 4) {
                decy = 0.0f;
            }
            Vector3D position = weaps->position;
            std::vector<Vector3D> path = {
                {0, -2*decy, 0},
                {decy, -decy, 0},
                {-decy,-decy,0}
            };

            for (int i = 0; i < weaps->nb_weap; i++) {
                glPushMatrix();
                glTranslatef(position.z/250+path[i].z, position.y/250 + path[i].y, -position.x/250+path[i].x);
                Renderer.DrawModel(weaps->objct, LOD_LEVEL_MAX);
                glPopMatrix();
                position.y -= 0.5f;

            }
        }
        glPopMatrix();
    }
}