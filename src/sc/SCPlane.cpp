#include  "precomp.h"

float tenthOfDegreeToRad(float angle) {
    return (angle/10) * (float(M_PI)/180.0f);
}

void gl_sincos(float a, float* b, float* c) {
	if (b != NULL) {
		*b = sinf(tenthOfDegreeToRad(a));
	}
	if (c != NULL) {
		*c = cosf(tenthOfDegreeToRad(a));
	}
}
int mrandom(int maxr) {
	static unsigned long randx = 1;
	register int n, retval;

	for (n = 1; n < 32 && (1 << n) < maxr; n++);

	retval = maxr << 1;
	while (retval > maxr) {
		randx = randx * 1103515245 + 12345;
		retval = (randx & 0x7fffffff) >> (31 - n);
	}
	randx = randx * 1103515245 + 12345;
	if (randx & 0x40000000)
		return (retval);
	else return (-retval);
}


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
SCPlane::SCPlane(
    float LmaxDEF,
    float LminDEF,
    float Fmax,
    float Smax,
    float ELEVF_CSTE,
    float ROLLFF_CSTE,
    float s,
    float W,
    float fuel_weight,
    float Mthrust,
    float b,
    float ie_pi_AR,
    int MIN_LIFT_SPEED,
    float pilot_y,
    float pilot_z,
    RSArea *area,
    float x,
    float y,
    float z
    ) {
    
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
    this->tps = 60;
    this->last_time = SDL_GetTicks();
    this->tick_counter = 0;
    this->last_tick = 0;
    this->x = x/COORD_SCALE;
    this->y = y/COORD_SCALE;
    this->z = z/COORD_SCALE;
    this->ro2 = .5f * ro[0];
    Init();
}
SCPlane::~SCPlane() {

}
void SCPlane::Init() {
    
    this->twist = 0;
    
    this->status = 580000;
    this->alive = this->tps << 2;
    this->flaps = 0;
    this->spoilers = 0;
    this->rollers = 0;
    this->rudder = 0;
    this->elevator = 0;

    this->ELEVF = this->ELEVF_CSTE * 10.0f / (20.0f * 400.0f);			/* elevator rate in degrees/sec	*/
    this->ROLLF = this->ROLLFF_CSTE  * 10.0f / (30.0f * 400.0f);			/* roll rate (both at 300 mph)	*/

    this->zdrag = 0.0f;
    this->ydrag = 0.0f;
    this->Cdp = .015f;				/* coefficient of parasitic drag*/
    this->ipi_AR = 1.0f / ((float) M_PI * this->b * this->b / this->s);	/* 1.0 / pi * wing Aspect Ratio	*/
    this->ie_pi_AR = this->ipi_AR / this->ie_pi_AR;		/* 1.0 / pi * AR * efficiency	*/
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
void SCPlane::Simulate() {
    int itemp;
    float temp;
    float elevtemp = 0.0f; 
    
    uint32_t current_time = SDL_GetTicks();
    uint32_t elapsed_time = (current_time - this->last_time) / 1000;
    if (elapsed_time > 1) {
        uint32_t ticks = this->tick_counter - this->last_tick;
        this->tps = ticks / elapsed_time;
        if (tps != 0) {
            this->last_time = current_time;
            this->last_tick = this->tick_counter;
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

    float groundlevel = this->area->getY(this->x*COORD_SCALE, this->z*COORD_SCALE)/COORD_SCALE;

    if (this->status > MEXPLODE) {
        /* tenths of degrees per tick	*/
        this->rollers = (this->ROLLF * ((this->control_stick_x + 8) >> 4) );
        itemp = (int) (this->rollers * this->vz - this->roll_speed);	/* delta	*/
        if (itemp != 0) {
            if (itemp >= DELAY || itemp <= -DELAY) {
                itemp /= DELAY;
            }
            else {
                itemp = itemp > 0 ? 1 : -1;
            }
        }
        if (this->wing_stall > 0) {
            itemp >>= this->wing_stall;
            itemp += mrandom(this->wing_stall << 3);
        }
        this->roll_speed += itemp;		
        this->elevator = -1.0f*(this->ELEVF * ((this->control_stick_y + 8) >> 4));
        itemp = (int)(this->elevator * this->vz + this->vy - this->elevation_speedf);
        elevtemp = this->elevator * this->vz + this->vy - this->elevation_speedf;
        if (itemp != 0) {
            if (itemp >= DELAY || itemp <= -DELAY) {
                itemp /= DELAY;
            }
            else {
                itemp = itemp > 0 ? 1 : -1;
            }
        }
        if (this->wing_stall > 0) {
            itemp >>= this->wing_stall;
            elevtemp = elevtemp / powf(2, this->wing_stall);
            itemp += mrandom(this->wing_stall *2);
            elevtemp += mrandom(this->wing_stall * 2);
        }
        this->elevation_speedf += itemp;
        float aztemp;
        temp = this->rudder * this->vz - (2.0f*this->tps/20.0f) * this->vx;
        if (this->on_ground) {
            itemp = (int)(16.0f * temp);
            if (itemp < -MAX_TURNRATE || itemp > MAX_TURNRATE) {
                /* clip turn rate	*/
                if (itemp < 0) {		
                    itemp = -MAX_TURNRATE;
                }
                else {
                    itemp = MAX_TURNRATE;
                }
                /* decrease with velocity */
                if (fabs(this->vz) > 10.0f / this->tps) {
                    /* skid effect */
                    temp = 0.4f * this->tps * (this->rudder * this->vz - .75f); 
                    if (temp < -MAX_TURNRATE || temp > MAX_TURNRATE) {
                        temp = (float) itemp;
                    }
                    itemp -= (int)temp;
                }
            }
            temp = (float)itemp;
        } else {
            itemp = (int) temp;	/* itemp is desired azimuth speed	*/
        }
        
        aztemp = temp;
        /* itemp is now desired-actual		*/
        itemp -=  (int) this->azimuth_speedf;	
        aztemp -= this->azimuth_speedf;
        if (itemp != 0) {
            if (itemp >= DELAY || itemp <= -DELAY) {
                itemp /= DELAY;
            }
            else {
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
            }
            else {		
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
        if (this->tick_counter%100 == 0) {
            // rebuild old ptw	
            // to keep it normalized
           
            this->ptw.Identity();
            this->ptw.translateM(this->x, this->y, this->z);
            
            this->ptw.rotateM(tenthOfDegreeToRad(this->azimuthf), 0, 1, 0);
            this->ptw.rotateM(tenthOfDegreeToRad(this->elevationf), 1, 0, 0);
            this->ptw.rotateM(tenthOfDegreeToRad(this->twist), 0, 0, 1);
        }
        this->ptw.translateM(this->vx, this->vy, this->vz);
        if (round(this->azimuth_speedf) != 0) this->ptw.rotateM(tenthOfDegreeToRad(roundf(this->azimuth_speedf)), 0, 1, 0);
        if (round(this->elevation_speedf) != 0) this->ptw.rotateM(tenthOfDegreeToRad(roundf(this->elevation_speedf)), 1, 0, 0);
        if (round(this->roll_speed) != 0) this->ptw.rotateM(tenthOfDegreeToRad((float) this->roll_speed), 0, 0, 1);


        /* analyze new ptw	*/
        temp = 0.0f;
        this->elevationf = (-asinf(this->ptw.v[2][1]) * 180.0f / (float) M_PI) * 10;
        
        float ascos = 0.0f;

        temp = cosf(tenthOfDegreeToRad(this->elevationf));
        
        if (temp != 0.0) {
            
            float sincosas = this->ptw.v[2][0] / temp;

            if (sincosas > 1) {
                sincosas = 1.0f;
            }
            else if (sincosas < -1) {
                sincosas = -1;
            }
            this->azimuthf = (asinf(sincosas) *180.0f / (float) M_PI) * 10.0f;
            if (this->ptw.v[2][2] < 0.0) {
                /* if heading into z	*/
                
                this->azimuthf = 1800 - this->azimuthf;	
            }	
            if (this->azimuthf < 0) {
                /* then adjust		*/
                this->azimuthf += 3600;
            }

            this->twist = (asinf(this->ptw.v[0][1] / temp) * 180.0f / (float) M_PI) * 10.0f;
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
        if (this->roll_speed) this->incremental.rotateM(tenthOfDegreeToRad((float) -this->roll_speed), 0, 0, 1);
        if (this->elevation_speedf) this->incremental.rotateM(tenthOfDegreeToRad(-this->elevation_speedf), 1, 0, 0);
        if (this->azimuth_speedf) this->incremental.rotateM(tenthOfDegreeToRad(-this->azimuth_speedf), 0, 1, 0);
        this->incremental.translateM(this->vx, this->vy, this->vz);


        this->vx = this->incremental.v[3][0];
        this->vy = this->incremental.v[3][1];
        this->vz = this->incremental.v[3][2];

        /****************************************************************/
        /*	check altitude for too high, and landing/takeoff            */
        /****************************************************************/
        /* flame out		*/
        if (this->y > 50000.0) this->thrust = 0;	
        else if (this->y > groundlevel + 4.0) {
            /* not on ground	*/
            
            if (this->on_ground) {
                this->Cdp /= 3.0;	
                this->min_throttle = 0;		
            }
            this->on_ground = FALSE;
        }
        else if (this->y < groundlevel) {
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
                        rating = report_card(-this->climbspeed, this->twist, (int)(this->vx * this->tps), (int)(-this->vz * this->fps_knots), this->wheels);
                        /* oops - you crashed	*/
                        if (rating == -1) {
                            /* set to exploding	*/
                            this->status = MEXPLODE;
                            /* increment count	*/
                            this->lost++;
                        }
                        else {
                            this->fuel += rating << 7;
                            if (this->fuel > (100 << 7))  this->fuel = 100 << 7;
                            this->max_throttle = 100;
                        }
                    }
                else {
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
    }	/* end not crashing	*/



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
        } 
        else {
            this->kl = (this->y / this->b);
            if (this->kl > .225f ) {
                this->kl = .484f * this->kl + .661f;
            }
            else {
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
                
            }
            else if (this->Cl < this->min_cl) {	
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
            }
            else {
                this->wing_stall = FALSE;
            }
        }
        else {
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
            }
            else {
                this->sos = -968.0f / this->tps;
            }
            itemp = ((int)this->y) >> 10;
            if (itemp > 74) {
                itemp = 74;
            }
            else if (itemp < 0) {
                itemp = 0;
            }
            this->ro2 = .5f * ro[itemp];
            if (this->Cl < .2) {
                this->mcc = .7166666f + .1666667f * this->Cl;
            }
            else {
                this->mcc = .7833333f - .1666667f * this->Cl;
            }
            /* and current mach number	*/
            this->mach = this->vz / this->sos;		
            this->mratio = this->mach / this->mcc;
            if (this->mratio < 1.034f) {
                this->Cdc = 0.0f;
            }
            else {
                this->Cdc = .082f * this->mratio - 0.084788f;
                if (this->Cdc > .03f) this->Cdc = .03f;
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
        }
        else if (this->lift < this->Lmin) {
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
        }
        else {
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
        this->climbspeed = (short) (this->tps * (this->y - this->last_py));


        this->vx += this->vx_add;
        this->vy += this->vy_add;
        this->vz += this->vz_add;
        if (this->thrust > 0) {
            itemp = this->thrust;
        }
        else {
            itemp = -this->thrust;
        }
        if (this->tick_counter%(100*TPS) == 1) {
            this->fuel_rate = fuel_consump(this->Mthrust, this->W);
            this->fuel -= (int)(itemp * this->fuel_rate);
            this->inverse_mass = G_ACC / (this->W + this->fuel / 12800.0f * this->fuel_weight);
        }
    }
    this->tick_counter ++;
}

int SCPlane::IN_BOX(int llx,int urx, int llz, int urz) {
    return (llx <= this->x && this->x <= urx && llz <= this->z && this->z <= urz);
}
int SCPlane::isOnRunWay() {
	
	for (int i = 0; i < area->objectOverlay.size(); i++) {
		
		if (IN_BOX(area->objectOverlay[i].lx, area->objectOverlay[i].hx, area->objectOverlay[i].ly, area->objectOverlay[i].hy)) {
			
			return 1;
		}
	}
	return 0;
}
float SCPlane::fuel_consump(float f, float b) {
	return  0.3f * f / b;
}
int SCPlane::report_card(int descent_rate, float roll_angle, int velocity_x, int velocity_z, int wheels_down) {
    int on_runway = isOnRunWay();
    int rating = 1;

    roll_angle /= 10;
    if (roll_angle > 180) roll_angle -= 360;

    if (!wheels_down) rating = 0;
    if (descent_rate > 100) rating = 0;
    if (roll_angle < 0) roll_angle = -roll_angle;
    if (roll_angle > 10) rating = 0;
    if (!on_runway) rating = 0;
    else if (velocity_x > 10 || velocity_x < -10) rating = 0;

    if (roll_angle > 20 || descent_rate > 20 || velocity_x > 20 || velocity_x < -20) rating = -1;

    return rating;
}

void SCPlane::SetThrottle(int throttle){
    this->thrust = throttle;
}
int SCPlane::GetThrottle(){
    return this->thrust;
}
void SCPlane::SetFlaps(int flaps) {
    this->flaps = flaps;
}
void SCPlane::SetSpoilers(int spoilers) {
    this->spoilers = spoilers;
}
void SCPlane::SetGears(int gears){
    this->wheels = gears;
}
void SCPlane::SetControlStick(int x, int y) {
    this->control_stick_x = x;
    this->control_stick_y = y;
}
void SCPlane::getPosition(Point3D* position) {
    position->x = this->x*COORD_SCALE;
    position->y = this->y*COORD_SCALE;
    position->z = this->z*COORD_SCALE;
}
void SCPlane::Render() {
    if (this->object != nullptr) {
        glPushMatrix();
        Matrix rotation;
        rotation.Clear();
        rotation.Identity();
        rotation.translateM(
            this->x*COORD_SCALE,
            this->y*COORD_SCALE,
            this->z*COORD_SCALE
        );
        rotation.rotateM(((this->azimuthf+900)/10.0f)*((float)M_PI/180.0f) , 0.0f, 1.0f, 0.0f);
        rotation.rotateM((this->elevationf/10.0f)*((float)M_PI/180.0f), 0.0f, 0.0f, 1.0f);
        rotation.rotateM(-(this->twist/10.0f)*((float)M_PI/180.0f), 1.0f, 0.0f, 0.0f);
        
        glMultMatrixf((float *)rotation.v);
        
        Renderer.DrawModel(this->object->entity, LOD_LEVEL_MAX);
        glPopMatrix();
    }
}