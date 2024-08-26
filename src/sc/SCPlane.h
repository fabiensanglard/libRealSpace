#ifndef __libRealSpace__SCPlane__
#define __libRealSpace__SCPlane__



#define  NAME_LENGTH  8
static float ro[75] =
{
	.0023081f,		/* 1000 feet	*/
	.0022409f,
	.0021752f,
	.0021110f,
	.0020482f,		/* 5000 feet	*/
	.0019869f,
	.0019270f,
	.0018685f,
	.0018113f,
	.0017556f,		/* 10000 feet	*/
	.0017011f,
	.0016480f,
	.0015961f,
	.0015455f,
	.0014962f,		/* 15000 feet	*/
	.0014480f,
	.0014011f,
	.0013553f,
	.0013107f,
	.0012673f,		/* 20000 feet	*/
	.0012249f,
	.0011836f,
	.0011435f,
	.0011043f,
	.0010663f,		/* 25000 feet	*/
	.0010292f,
	.00099311f,
	.00095801f,
	.00092387f,
	.00089068f,		/* 30000 feet	*/
	.00085841f,
	.00082704f,
	.00079656f,
	.00076696f,
	.00073820f,		/* 35000 feet	*/
	.00071028f,
	.00067800f,
	.00064629f,
	.00061608f,
	.00058727f,		/* 40000 feet	*/
	.00055982f,
	.00053365f,
	.00050871f,
	.00048493f,
	.00046227f,		/* 45000 feet	*/
	.00044067f,
	.00042008f,
	.00040045f,
	.00038175f,
	.00036391f,		/* 50000 feet	*/
	.00034692f,
	.00033072f,
	.00031527f,
	.00030055f,
	.00028652f,		/* 55000 feet	*/
	.00027314f,
	.00026039f,
	.00024824f,
	.00023665f,
	.00022561f,		/* 60000 feet	*/
	.00021508f,
	.00020505f,
	.00019548f,
	.00018336f,
	.00017767f,		/* 65000 feet	*/
	.00016938f,
	.00016148f,
	.00015395f,
	.00014678f,
	.00013993f,		/* 70000 feet	*/
	.00013341f,
	.00012719f,
	.00012126f,
	.00011561f,
	.00011022f,		/* 75000 feet	*/
};

#define MEXPLODE 0
#define TPS 20
#define MAX_TURNRATE (600/TPS)
#define DELAY_FACTOR 4
#define DELAY TPS/4
#define DELAYF TPS/4.0f


#define G_ACC 32.17f

#define COORD_SCALE 0.36f
class SCPlane {

private:
    long planeid;
    /* flight version	*/
    char  version;
    /* type of packet	*/
    char  cmd;
    /* plane type		*/
    short type;
    short alive;
    char  myname[NAME_LENGTH + 1];
    unsigned int status;
    /* for msgs these 2 shorts */
    unsigned short won;
    /* hold the plane id	*/
    unsigned short lost;

    /* plane position	*/
    


    /*roll, elevation, azimuth speeds	* /
    /* in 10'ths degrees per tick	*/
    int roll_speed;

    

    /* roll, elevation, azimuth speeds	*/
    /* in 10'ths degrees per tick	*/
    float elevation_speedf;
    float azimuth_speedf;

    /* missile data		*/
    short mstatus;
    float mx;
    float my;
    float mz;
    float last_mx;
    float last_my;
    float last_mz;
    long mkill;
    
    int thrust;


    char mtype;

    /* rollers position 	*/
    float rollers;
    float rudder;
    /* elevator position 	*/
    float elevator;
    float ELEVF_CSTE;
    float ROLLFF_CSTE;
    float LminDEF;
    float LmaxDEF;
    /* maximum flap deflection	*/
    float Fmax;
    float Smax;


    /* last plane position	*/
    float last_px;
    float last_py;
    float last_pz;
    /* plane acceleration	*/
    float ax;
    float ay;
    float az;
    /* drag force in y and z	*/
    float ydrag;
    float zdrag;
    /* fuel consumption rate	*/
    float fuel_rate;
    /* lift acceleration		*/
    float lift;
    /* maximum height for ground effect	*/
    float gefy;
    /* angle of attack for wing		*/
    float ae;
    /* max and min coefficient of lift	*/
    float max_cl;
    float min_cl;
    /* wing angle tilt due to flaps		*/
    float tilt_factor;
    /* spoiler factors on lift and drag	*/
    float Splf;
    float Spdf;
    /* air density / 2.0, speed of sound	*/
    float ro2;
    float sos;
    /* mach #, crest critical #, ratio	*/
    float mach;
    float mcc;
    float mratio;
    /* coefficients of lift and drag	*/
    float uCl;
    float Cl;
    float Cd;
    float Cdc;
    float kl;
    /* ground effect, ro/2*Vz*s		*/
    float qs;
    /* weight of fuel			*/
    float fuel_weight;
    /* 1.0 / mass of plane			*/
    float inverse_mass;
    /* plane design parameters	*/
    float s;
    float W;
    float Mthrust;
    float b;
    float Cdp;
    float ipi_AR;
    float ie_pi_AR;
    float Lmax;
    float Lmin;
    float ELEVF;
    float ROLLF;
    float pilot_y;
    float pilot_z;
    float last_zdrag;

    short val;
    /* TRUE if the wheels are down	*/
    short wheels;
    /* wheel position 	*/
    /* used only by F16W		*/
    short wheels_retracting;
    /* >= 0 if the gear is stuck	*/
    short landing_gear_stuck;
    /* TRUE in autopilot mode	*/
    short autopilot;
    /* TRUE if wing g-limit is hit	*/
    short g_limit;
    /* fuel (0 - 12800)		*/
    short fuel;
    /* upper limit on engines	*/
    short max_throttle;
    /* lower limit on engines	*/
    short min_throttle;
    /* max rockets and sidewinders	*/
    short MAX_RK;
    short MAX_SW;
    /* minimum lift-up speed fps	*/
    short MIN_LIFT_SPEED;
    short last_airspeed;
    short target_speed;
    short climbspeed;
    short last_climbspeed;
    short target_climb;
    /* TRUE if wing is stalling	*/
    short wing_stall;
    /* flap and spoiler settings	*/
    int flaps;
    int spoilers;
    int obj;
    
    /* missile velocity */
    float missile_vx;
    float missile_vy;
    float missile_vz;
    /* fps to knots conversion factor */
    float fps_knots;
    /* the effect of gravity realtive to tps */
    float gravity;
    /* number of armaments		*/
    int sidewinders, rockets;
    
    /* my ptw matrix, temp matrix	*/
    Matrix ptw;
    Matrix incremental;

    RSArea *area;
    RSEntity *object;

    float vx_add;
    float vy_add;
    float vz_add;

    

    int nocrash;
    
    
    int tps;

    int IN_BOX(int llx,int urx, int llz, int urz);
    int report_card(int descent, int roll, int vx, int vz, int wheels);
    int isOnRunWay();
    float fuel_consump(float f, float b);

    uint32_t last_time;
    uint32_t tick_counter;
    uint32_t last_tick;
public:

    /* plane velocity */
    float vx;
    float vy;
    float vz;

    float x;
    float y;
    float z;

    int control_stick_x;
    int control_stick_y;

    short on_ground;

    int airspeed;
    short twist;
    float azimuthf;
    float elevationf;

    SCPlane();
    SCPlane(
        float LmaxDEF,
        float LminDEF,
        float Fmax,
        float Smax,
        float ELEVF_CSTE,
        float ROLLFF_CSTE,
        int obj,
        float s,
        double W,
        float fuel_weight,
        double Mthrust,
        float b,
        float ie_pi_AR,
        int MAX_RK,
        int MAX_SW,
        int MIN_LIFT_SPEED,
        float pilot_y,
        float pilot_z,
        RSArea *area,
        float x,
        float y,
        float z
    );
    ~SCPlane();
    void Init();
    void SetThrottle(int throttle);
    int GetThrottle();
    void SetFlaps(int flaps);
    void SetSpoilers(int spoilers);
    void SetGears(int gears);
    void SetControlStick(int x, int y);
    void Simulate();
    void getPosition(Point3D* position);
};

#endif