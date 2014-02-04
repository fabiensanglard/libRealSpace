//
//  Backgrounds.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef libRealSpace_Backgrounds_h
#define libRealSpace_Backgrounds_h

//CONVSHPS.PAK
enum ConvShapeID {
    

DESERT_LANDSTRIP,
HACEIENDA,
wc_hangar=7,
    tent=9,
    rich_mansion_entry=12,
    wc_changeroom=13,
    office=15,
    office2=16,
    wc_hangar2=19, //Wildcat hangar conv
    wc_hangar3=20, //Wildcat hangar conv
    tent2=21,
    tent3=22,
    virgil_office_chair=23,
    virgil_office_wall=24,
    wc_changeroom2=25,
    wc_changeroom_fullscreen=26, //full screen
    prideauxCorner_chair = 29,
    prideauxCorner_wall = 30,
    desert_truck = 31,
    desert_truck2 =32,
    shelves = 33,
    shelves2= 34,
    backalley=35,
    windows = 36,
    base_cu = 38,
    ba2_cu= 39,
    set_windows2=40,
    set_map=41,
    bar_fs=42,
    bar2_fs=43,
    bar_counter=44,
    bar_counter2=45,
    
    //FACE
    AIR, //46    NOt a PAK, a sequence of 62 entries.
          /*
         HAIRS
         ANIMS
         cloths
         sunclass
         helmet
         visor
         //palette ?
           */
    BETA,     // 47
          //last entry = accept_full screen
    BILLY,  //48
    FARID, //49
          //not last entry = accept_full screen
    GWEN,  //50
    JANET, //51 betray me girl
    LYLE,
    MELE = 53,
    MIGUEL =54,
    MULSTAFA =55,
    prideau =56,
    stern =57,
    tex =58,
    virgil =59,
    walt =60,
    tweedly =61,
    goon =62,
    
    
    //FIGURE
    FIGURE_AIR = 63,
    
    FIGURE_X = 63,
    
    FIGURE_STERN = 68,
    
    FIGURE_MIGUEL = 70,
    
    FIGURE_VIRGIL = 71,
    
    FIGURE_GWEN = 72,
    
    FIGURE_GENERAK = 74,
    FIGURE_LYLE = 75,
    FIGURE_TEX = 76,
    FIGURE_VIRGIL2 = 77,
    
    FACE_MUGGER = 78,
    
    WILD_CAT_BASE_DESTROYED= 79, //full screen
    DONNO= 80, //full screen
    
    ALL_FIGURES = 81,
    TEX_FIGURE = 82,
    GWEN_FIGURE = 83,
     X_FIGURE = 84,
     Y_FIGURE = 85,
     Z_FIGURE = 86,
    
    AIRPLANE_WILDCAT_BACKGROUND = 87,
    AIRPLACE_OUTDOOR_BACKGROUND = 88,
    
    FIGURE_AIR_POINTING = 89, // animation
    
    AIRPLANE_WILDCAT_BACKGROUND2 = 90,
    AIRPLANE_OUTDOOR_BACKGROUND2 = 91,
    
    BANKRUPT_VIRGIL_OFFICE = 92,
    
   
};


//OPTSHPS.PAK
enum OptionShapeID{
    BAR_ROOM_PRUDEAUX = 0,
    BAR_LOBBY_BILLY = 1,
    BAR_ROOM_FAN = 2, // animation 6 images
    BAR_LOBBY_JANE = 4, // animation 8 images
    BAR_LOBBY_MIGUEL = 5, // animation 12 images
    BAR_ROOM_QUATARIS = 6, // animation 14 images
    BAR_LOBBY_WC_MEMBER = 7, // animation 11 images
    BAR_BACKROOM_BG = 8,
    BAR_LOBBY_BG = 9,
    BAR_ROOM_BG = 10,
    BAR_ROOM_BURRITO_MAN = 11, // animation 10 images
    BAR_ROOM_UNKNOWN_MAN = 12, // animation 13 images
    WILDCAT_CHANGEROOM_BG = 13,
    
    WILDCAT_HANGAR_VEHICULE_F16 = 16,
    SOMETHING = 17,
    
    WILDCAT_HANGAR_VEHICULE_JEEP = 17, //also contains the animation, not in a pak
    
    WILDCAT_HANGAR = 18,
    WILDCAT_HANGAR_DAMAGE_DECAL = 19,
    WILDCAT_HANGAR_VEHICULE_TRUCK = 20, //also contains the animation, not in a pak
    WILDCAT_VIRGIL_OFFICE = 21,
    WILDCAT_VIRGIL_OFFICE_VIRGIL = 22, // animation 35 images
    ISTANBUL_BG = 23,
    BAR_FROM_WINDOW_BG = 24,
    MISSION_TENT_BG = 25,
    
    WILDCAT_CHANGEROOM_ANIM = 26, // animation 10  images
    MISSION_TENT_LEFT_WOMAN = 27, // animation 12  images
    BAR_LOBBY_GWEN = 28, // animation 9 images
    MISSION_TENT_MAN_MAP = 29, // animation 6 images
    MISSION_TENT_STERN_SEAT = 30, // animation 5 images
    MISSION_TENT_MIGUEL_SEAT = 31, // animation 5 images
    MISSION_TENT_BILLY = 32, // animation 6 images
    
    WILDCAT_HANGAR_GWEN = 35, // animation 5 images
    WILDCAT_HANGAR_JANE = 36, // animation 9 images
    WILDCAT_HANGAR_MIGUEL = 37, // animation 13 images
    
    WILDCAT_HANGAR_LYLE = 39, // animation 7 images
    
    SELECT_WEAPON_F16_IN_DESERT_BG = 40,
    
    ACCOUNTING_BOOK = 41, //animation (not in a pak) 5 images
    
    DESERT_TAKE_OFF_BG = 42,
    
    ISTANBUL_TO_BAR_ROAD_CITY_BG = 43,
    
    MOUTAINS_BG = 44,
    
    LAMDSTRIP_BG = 45,
    
    SELECT_WEAPON_F16_DECAL = 47,
    
    SELECT_WEAPON_F16_AT_WILDCAT_BG = 91,
    
    ACCOUNTING_BOOK_STATIC = 92,
    ACCOUNTING_DECALS = 93, // UP TO 114
    
    MISSION_DESERT_OUTSIDE_BG = 115,
    
    SKY = 116,
    BAR_SALIMS_OUTSIDE = 117,
    
    
    WILDCAT_CHANGEROOM_PINUP_W = 118,
    WC_BASE_OUTSIDE_BG = 119,
    LAMDSTRIP_BG2 = 120,
    
    FROM_SALIM_BAR_CAR_DEPART_ANIM = 121, //animation (not in a pak) 21 images
    FROM_SALIM_FRONT_CAR_DEPART_ANIM = 122, //animation (not in a pak) 21 images
    
    MAP_NORTH_AMERICA = 128,
    MAP_SOUTH_AMERICA = 129,
    MAP_EUROPE = 130,
    MAP_SOUTH_AFRICA = 131,
    MAP_ISLAND = 132,
    
    MAP_ALASKA = 134,
    
    ANIM_PLANE = 135,
    
    
    FROM_SALIM_CAR_DEPART_ANIM = 136, //animation (not in a pak) 20 images
    
    TO_SALIM_BAR_CAR_GOING_ANIM = 137, //animation (not in a pak)
    TO_SALIM_FRONT_CAR_GOING_ANIM = 138, //animation (not in a pak)
    TO_WILCAT_BASE_CAR_GOING_ANIM = 139, //animation (not in a pak)
    
    JET_SIDE_PAINT_SHOTS = 141,
    JET_SIDE_PAINT_SHOTS_ITEMS= 142, //not a pak
                            //planes
                            //planes5
                            //planes10
                            //ground
                            //ground5
                            //ground10
    
    JET_SIDE_PAINT_SHOTS_ANIMATION= 143, //(not in a pak)
    
    JUNGLE_TREES_BG = 144,
    
    TUNDRA_TREES_BG = 145,
    MISSION_OUTSIDE_VEHICULE_TRUCK = 146,
    MISSION_OUTSIDE_VEHICULE_F16 = 147,
    MISSION_OUTSIDE_VEHICULE_F22 = 148,
    DESERT_LANDSCAPE_OUTSIDE_BG = 149,
    
    START_GAME_REGISTRATION = 160,
    
    BAR_ROOM_SUNGLASSES_MAN = 161,
    
    LANDING_TRIP_SUMMER = 169,
    LANDING_TRIP_WINTER = 170,
    LANDING_TRIP_SPRING = 171,
    
    MOUNTAIN_ANIM_BG = 171,
    C130_WITH_ONE_f16 = 172,
    WILDCAT_CHANGEROOM_PINUP_M = 174,
    //NO MORE IMAGES AFTER THAT
};
#endif
