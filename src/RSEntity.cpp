//
//  RSEntity.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//


#include <cfloat>
#include <algorithm>
#include "RSEntity.h"

RSEntity::RSEntity(AssetManager *amana) {
    this->assetsManager = amana;
}

RSEntity::~RSEntity() {
    while (!images.empty()) {
        RSImage *image = images.back();
        images.pop_back();
        delete image;
    }
}

void RSEntity::InitFromRAM(uint8_t *data, size_t size) {

    IFFSaxLexer lexer;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["REAL"] = std::bind(&RSEntity::parseREAL, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
    CalcBoundingBox();
}

BoudingBox *RSEntity::GetBoudingBpx(void) { return &this->bb; }

void RSEntity::CalcBoundingBox(void) {

    this->bb.min.x = FLT_MAX;
    this->bb.min.y = FLT_MAX;
    this->bb.min.z = FLT_MAX;

    this->bb.max.x = FLT_MIN;
    this->bb.max.y = FLT_MIN;
    this->bb.max.z = FLT_MIN;

    for (size_t i = 0; i < this->vertices.size(); i++) {

        Point3D vertex = vertices[i];

        if (bb.min.x > vertex.x)
            bb.min.x = vertex.x;
        if (bb.min.y > vertex.y)
            bb.min.y = vertex.y;
        if (bb.min.z > vertex.z)
            bb.min.z = vertex.z;

        if (bb.max.x < vertex.x)
            bb.max.x = vertex.x;
        if (bb.max.y < vertex.y)
            bb.max.y = vertex.y;
        if (bb.max.z < vertex.z)
            bb.max.z = vertex.z;
    }
}

size_t RSEntity::NumImages(void) { return this->images.size(); }

size_t RSEntity::NumVertice(void) { return this->vertices.size(); }

size_t RSEntity::NumUVs(void) { return this->uvs.size(); }

size_t RSEntity::NumLods(void) { return this->lods.size(); }

size_t RSEntity::NumTriangles(void) { return this->triangles.size(); }

void RSEntity::AddImage(RSImage *image) { this->images.push_back(image); }

void RSEntity::AddVertex(Point3D *vertex) { this->vertices.push_back(*vertex); }

void RSEntity::AddUV(uvxyEntry *uv) { this->uvs.push_back(*uv); }

void RSEntity::AddLod(Lod *lod) { this->lods.push_back(*lod); }

void RSEntity::AddTriangle(Triangle *triangle) { this->triangles.push_back(*triangle); }

void RSEntity::parseREAL(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["OBJT"] = std::bind(&RSEntity::parseREAL_OBJT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["APPR"] = std::bind(&RSEntity::parseREAL_APPR, this, std::placeholders::_1, std::placeholders::_2);
    
    
    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSEntity::parseREAL_OBJT_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["AFTB"] = std::bind(&RSEntity::parseREAL_OBJT_AFTB, this, std::placeholders::_1, std::placeholders::_2);
    handlers["JETP"] = std::bind(&RSEntity::parseREAL_OBJT_JETP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["GRND"] = std::bind(&RSEntity::parseREAL_OBJT_GRND, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ORNT"] = std::bind(&RSEntity::parseREAL_OBJT_ORNT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["EXTE"] = std::bind(&RSEntity::parseREAL_OBJT_EXTE, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MISS"] = std::bind(&RSEntity::parseREAL_OBJT_MISS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["BOMB"] = std::bind(&RSEntity::parseREAL_OBJT_MISS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TRCR"] = std::bind(&RSEntity::parseREAL_OBJT_MISS, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_AFTB(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["EXTE"] = std::bind(&RSEntity::parseREAL_OBJT_EXTE, this, std::placeholders::_1, std::placeholders::_2);
    handlers["APPR"] = std::bind(&RSEntity::parseREAL_OBJT_AFTB_APPR, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_AFTB_APPR(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["AFTB"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}

void RSEntity::parseREAL_OBJT_MISS(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["EXPL"] = std::bind(&RSEntity::parseREAL_OBJT_MISS_EXPL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SIGN"] = std::bind(&RSEntity::parseREAL_OBJT_MISS_SIGN, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TRGT"] = std::bind(&RSEntity::parseREAL_OBJT_MISS_TRGT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SMOK"] = std::bind(&RSEntity::parseREAL_OBJT_MISS_SMOK, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DAMG"] = std::bind(&RSEntity::parseREAL_OBJT_MISS_DAMG, this, std::placeholders::_1, std::placeholders::_2);
    handlers["WDAT"] = std::bind(&RSEntity::parseREAL_OBJT_MISS_WDAT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DATA"] = std::bind(&RSEntity::parseREAL_OBJT_MISS_DATA, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DYNM"] = std::bind(&RSEntity::parseREAL_OBJT_MISS_DYNM, this, std::placeholders::_1, std::placeholders::_2);


    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_MISS_EXPL(uint8_t *data, size_t size){}
void RSEntity::parseREAL_OBJT_MISS_SIGN(uint8_t *data, size_t size){}
void RSEntity::parseREAL_OBJT_MISS_TRGT(uint8_t *data, size_t size){}
void RSEntity::parseREAL_OBJT_MISS_SMOK(uint8_t *data, size_t size){}
void RSEntity::parseREAL_OBJT_MISS_DAMG(uint8_t *data, size_t size){}
void RSEntity::parseREAL_OBJT_MISS_WDAT(uint8_t *data, size_t size){
    WDAT *wdat = new WDAT();
    ByteStream bs(data);
    wdat->damage = bs.ReadShort();
    wdat->radius = bs.ReadShort();
    wdat->unknown1 = bs.ReadByte();
    wdat->weapon_id = bs.ReadByte();
    wdat->weapon_category = bs.ReadByte();
    wdat->radar_type = bs.ReadByte();
    wdat->weapon_aspec = bs.ReadByte();
    wdat->target_range = bs.ReadInt32LE();
    wdat->tracking_cone = bs.ReadByte();
    wdat->effective_range = bs.ReadInt32LE();
    wdat->unknown6 = bs.ReadByte();
    wdat->unknown7 = bs.ReadByte();
    wdat->unknown8 = bs.ReadByte();
    this->wdat = wdat;
}
void RSEntity::parseREAL_OBJT_MISS_DATA(uint8_t *data, size_t size){}
void RSEntity::parseREAL_OBJT_MISS_DYNM(uint8_t *data, size_t size){
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["DYNM"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_DYNM, this, std::placeholders::_1, std::placeholders::_2);
    handlers["MISS"] = std::bind(&RSEntity::parseREAL_OBJT_MISS_DYNM_MISS, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_MISS_DYNM_MISS(uint8_t *data, size_t size){
    ByteStream bs(data);
    DYNN_MISS *dynn_miss = new DYNN_MISS();
    bs.ReadByte();
    dynn_miss->turn_degre_per_sec = bs.ReadInt24LE();
    dynn_miss->velovity_m_per_sec = bs.ReadInt24LE();
    this->dynn_miss = dynn_miss;
}
void RSEntity::parseREAL_OBJT_INFO(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    this->entity_type = EntityType::jet;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["EXPL"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_EXPL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DEBR"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DEBR, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DEST"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DEST, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SMOK"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_SMOK, this, std::placeholders::_1, std::placeholders::_2);

    handlers["CHLD"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_CHLD, this, std::placeholders::_1, std::placeholders::_2);
    handlers["JINF"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_JINF, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DAMG"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DAMG, this, std::placeholders::_1, std::placeholders::_2);
    handlers["EJEC"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_EJEC, this, std::placeholders::_1, std::placeholders::_2);

    handlers["SIGN"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_SIGN, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TRGT"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_TRGT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["CTRL"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_CTRL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TOFF"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_TOFF, this, std::placeholders::_1, std::placeholders::_2);

    handlers["LAND"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_LAND, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DYNM"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM, this, std::placeholders::_1, std::placeholders::_2);
    handlers["WEAP"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DAMG"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DAMG, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_ORNT(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    this->entity_type = EntityType::ornt;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["EXPL"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_EXPL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DEBR"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DEBR, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DEST"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DEST, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SMOK"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_SMOK, this, std::placeholders::_1, std::placeholders::_2);

    handlers["CHLD"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_CHLD, this, std::placeholders::_1, std::placeholders::_2);
    handlers["JINF"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_JINF, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DAMG"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DAMG, this, std::placeholders::_1, std::placeholders::_2);
    handlers["EJEC"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_EJEC, this, std::placeholders::_1, std::placeholders::_2);

    handlers["SIGN"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_SIGN, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TRGT"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_TRGT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["CTRL"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_CTRL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TOFF"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_TOFF, this, std::placeholders::_1, std::placeholders::_2);

    handlers["LAND"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_LAND, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DYNM"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM, this, std::placeholders::_1, std::placeholders::_2);
    handlers["WEAP"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DAMG"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DAMG, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_GRND(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;
    this->entity_type = EntityType::ground;
    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["EXPL"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_EXPL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DEBR"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DEBR, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DEST"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DEST, this, std::placeholders::_1, std::placeholders::_2);
    handlers["SMOK"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_SMOK, this, std::placeholders::_1, std::placeholders::_2);

    handlers["CHLD"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_CHLD, this, std::placeholders::_1, std::placeholders::_2);
    handlers["JINF"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_JINF, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DAMG"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DAMG, this, std::placeholders::_1, std::placeholders::_2);
    handlers["EJEC"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_EJEC, this, std::placeholders::_1, std::placeholders::_2);

    handlers["SIGN"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_SIGN, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TRGT"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_TRGT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["CTRL"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_CTRL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TOFF"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_TOFF, this, std::placeholders::_1, std::placeholders::_2);

    handlers["LAND"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_LAND, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DYNM"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM, this, std::placeholders::_1, std::placeholders::_2);
    handlers["WEAP"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DAMG"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DAMG, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}

void RSEntity::parseREAL_OBJT_JETP_EXPL(uint8_t *data, size_t size) {
    EXPL *expl = new EXPL();
    
    std::string tmpname;
    ByteStream bs(data);
    expl->name = bs.ReadString(8);
    tmpname = "..\\..\\DATA\\OBJECTS\\" + expl->name + ".IFF";
    expl->x = bs.ReadShort();
    expl->y = bs.ReadShort();
    expl->objct = new RSEntity(this->assetsManager);
    TreEntry *entry = assetsManager->GetEntryByName(tmpname);
    expl->objct->InitFromRAM(entry->data, entry->size);
}
void RSEntity::parseREAL_OBJT_JETP_DEBR(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DEST(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_SMOK(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_CHLD(uint8_t *data, size_t size) {
    size_t nb_child = size /32;
    ByteStream bs(data);
    for (size_t i = 0; i < nb_child; i++) {
        CHLD *chld = new CHLD();
        
        std::string tmpname = bs.ReadString(8);
        std::transform(tmpname.begin(), tmpname.end(), tmpname.begin(), ::toupper);
        chld->name ="..\\..\\DATA\\OBJECTS\\"+tmpname+".IFF";
        chld->x = bs.ReadInt32LE();
        chld->z = bs.ReadInt32LE();
        chld->y = bs.ReadInt32LE();
        
        for (size_t j = 0; j < 12; j++) {
            chld->data.push_back(bs.ReadByte());
        }
        RSEntity *objct = new RSEntity(this->assetsManager);
        
        TreEntry *entry = assetsManager->GetEntryByName(chld->name);
        if (entry != nullptr) {
            objct->InitFromRAM(entry->data, entry->size);
            chld->objct = objct;
            this->chld.push_back(chld);
        }
    }
}
void RSEntity::parseREAL_OBJT_JETP_JINF(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DAMG(uint8_t *data, size_t size) {
    if (size > 2) {
        IFFSaxLexer lexer;

        std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
        handlers["SYSM"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP_DAMG_SYSM, this, std::placeholders::_1, std::placeholders::_2);

        lexer.InitFromRAM(data, size, handlers);
    }
}
void RSEntity::parseREAL_OBJT_JETP_EJEC(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_SIGN(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_TRGT(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_CTRL(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_TOFF(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_LAND(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["DYNM"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_DYNM, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ORDY"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_ORDY, this, std::placeholders::_1, std::placeholders::_2);
    handlers["STBL"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_STBL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ATMO"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_ATMO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["GRAV"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_GRAV, this, std::placeholders::_1, std::placeholders::_2);
    handlers["THRS"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_THRS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["JDYN"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_JDYN, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_JETP_DYNM_DYNM(uint8_t *data, size_t size) {
    ByteStream bs(data);
    if (size == 4) {
        bs.ReadByte();
        this->weight_in_kg = bs.ReadInt24LEByte3();
    }
}
void RSEntity::parseREAL_OBJT_JETP_DYNM_ORDY(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM_STBL(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM_ATMO(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM_GRAV(uint8_t *data, size_t size) {
    this->gravity = true;
}
void RSEntity::parseREAL_OBJT_JETP_DYNM_THRS(uint8_t *data, size_t size) {
    ByteStream bs(data);
    bs.ReadByte();
    this->thrust_in_newton = bs.ReadInt24LEByte3();
    /* il reste 2 ou 3 octets (en fonction de la taille du chunk 6 ou 7) en suite dont je ne sais rien*/
}
void RSEntity::parseREAL_OBJT_JETP_DYNM_JDYN(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_WEAP(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DCOY"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP_DCOY, this, std::placeholders::_1, std::placeholders::_2);
    handlers["WPNS"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP_WPNS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["HPTS"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP_HPTS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DAMG"] =
        std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP_DAMG, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_JETP_WEAP_INFO(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_WEAP_DCOY(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_WEAP_WPNS(uint8_t *data, size_t size) {
    size_t nb_weap = 0;
    nb_weap = size / 10;
    ByteStream bs(data);  

    for (size_t i = 0; i < nb_weap; i++) {
        WEAPS *htps = new WEAPS();
        htps->nb_weap = bs.ReadShort();
        htps->name = bs.ReadString(8);
        std::transform(htps->name.begin(), htps->name.end(), htps->name.begin(), ::toupper);
        std::string tmpname = "..\\..\\DATA\\OBJECTS\\" + htps->name + ".IFF";
        RSEntity *objct = new RSEntity(this->assetsManager);
        TreEntry *entry = assetsManager->GetEntryByName(tmpname);
        if (entry != nullptr) {
            objct->InitFromRAM(entry->data, entry->size);
            htps->objct = objct;
        }
        this->weaps.push_back(htps);
    }
}
void RSEntity::parseREAL_OBJT_JETP_WEAP_HPTS(uint8_t *data, size_t size) {
    size_t nb_hpts = size / 13;
    ByteStream bs(data);
    for (size_t i = 0; i < nb_hpts; i++) {
        HPTS *hpts = new HPTS();
        hpts->id = bs.ReadByte();
        hpts->x = bs.ReadInt32LE();
        hpts->z = bs.ReadInt32LE();
        hpts->y = bs.ReadInt32LE();
        this->hpts.push_back(hpts);
    }
}
void RSEntity::parseREAL_OBJT_JETP_WEAP_DAMG(uint8_t *data, size_t size) {
    
}
void RSEntity::parseREAL_OBJT_JETP_WEAP_DAMG_SYSM(uint8_t *data, size_t size) {
    ByteStream bs(data);

    this->life = bs.ReadShort();
    size_t nb_sysm = (size - 2) / 18;
    
    for (size_t i=0; i<nb_sysm;i++) {
        uint16_t pv = bs.ReadShort();
        std::string sub_system = bs.ReadString(8);
        std::string main_system = bs.ReadString(8);
        this->sysm[main_system][sub_system] = pv;
    }
}
void RSEntity::parseREAL_OBJT_EXTE(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_APPR(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["POLY"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    handlers["AFTB"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_APPR_POLY(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["VERT"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_VERT, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DETA"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_DETA, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TRIS"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_TRIS, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_APPR_POLY_INFO(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_APPR_POLY_VERT(uint8_t *data, size_t size) {
    ByteStream stream(data);
    size_t numVertice = size / 12;

    for (int i = 0; i < numVertice; i++) {
        int32_t coo;
        Point3D vertex;

        coo = stream.ReadInt32LE();
        vertex.z = (coo >> 8) + static_cast<float>(coo & 0x000000FF) / 255.0f;

        coo = stream.ReadInt32LE();
        vertex.x = (coo >> 8) + static_cast<float>(coo & 0x000000FF) / 255.0f;

        coo = stream.ReadInt32LE();
        vertex.y = (coo >> 8) + static_cast<float>(coo & 0x000000FF) / 255.0f;

        AddVertex(&vertex);
    }
}
void RSEntity::parseREAL_APPR_POLY_DETA(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["LVL0"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_DETA_LVLX, this, std::placeholders::_1, std::placeholders::_2);
    handlers["LVL1"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_DETA_LVLX, this, std::placeholders::_1, std::placeholders::_2);
    handlers["LVL2"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_DETA_LVLX, this, std::placeholders::_1, std::placeholders::_2);
    handlers["LVL3"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_DETA_LVLX, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_APPR_POLY_DETA_LVLX(uint8_t *data, size_t size) {
    ByteStream stream(data);
    Lod lod;

    lod.numTriangles = static_cast<uint16_t>((size - 4) / 2);
    lod.dist = stream.ReadUInt32LE();

    for (int i = 0; i < lod.numTriangles; i++) {
        lod.triangleIDs[i] = stream.ReadUShort();
    }

    AddLod(&lod);
}
void RSEntity::parseREAL_APPR_POLY_TRIS(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["VTRI"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_TRIS_VTRI, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TXMS"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_TRIS_TXMS, this, std::placeholders::_1, std::placeholders::_2);
    handlers["UVXY"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_TRIS_UVXY, this, std::placeholders::_1, std::placeholders::_2);

    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_APPR_POLY_TRIS_VTRI(uint8_t *data, size_t size) {

    size_t numTriangle = size / 8;
    ByteStream stream(data);

    Triangle triangle;
    for (int i = 0; i < numTriangle; i++) {

        triangle.property = stream.ReadByte();

        triangle.ids[0] = stream.ReadByte();
        triangle.ids[1] = stream.ReadByte();
        triangle.ids[2] = stream.ReadByte();

        triangle.color = stream.ReadByte();

        triangle.flags[0] = stream.ReadByte();
        triangle.flags[1] = stream.ReadByte();
        triangle.flags[2] = stream.ReadByte();

        AddTriangle(&triangle);
    }
}
void RSEntity::parseREAL_APPR_POLY_TRIS_TXMS(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_TRIS_TXMS_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TXMP"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_TRIS_TXMS_TXMP, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TXMA"] =
        std::bind(&RSEntity::parseREAL_APPR_POLY_TRIS_TXMS_TXMP, this, std::placeholders::_1, std::placeholders::_2);
    lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_APPR_POLY_TRIS_TXMS_INFO(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_APPR_POLY_TRIS_TXMS_TXMP(uint8_t *data, size_t size) {
    ByteStream stream(data);

    RSImage *image = new RSImage();

    char name[9];

    for (int i = 0; i < 8; i++)
        name[i] = stream.ReadByte();

    uint32_t width = stream.ReadShort();
    uint32_t height = stream.ReadShort();

    image->Create(name, width, height, 0);
    uint8_t *src = stream.GetPosition();
    uint8_t *pic_data = nullptr;
    size_t csize = 0;
    if (src[0]=='L' && src[1]=='Z'){
        LZBuffer lzbuffer;
        pic_data = lzbuffer.UncompressLZWTextbook(src+2,size-14,csize);
    }
    if (pic_data != nullptr && csize>0 && csize == width * height) {
        image->UpdateContent(pic_data);
        AddImage(image);
    }
}
void RSEntity::parseREAL_APPR_POLY_TRIS_UVXY(uint8_t *data, size_t size) {
    ByteStream stream(data);

    size_t numEntries = size / 8;

    uvxyEntry uvEntry;
    for (size_t i = 0; i < numEntries; i++) {

        uvEntry.triangleID = stream.ReadByte();
        uvEntry.textureID = stream.ReadByte();

        uvEntry.uvs[0].u = stream.ReadByte();
        uvEntry.uvs[0].v = stream.ReadByte();

        uvEntry.uvs[1].u = stream.ReadByte();
        uvEntry.uvs[1].v = stream.ReadByte();

        uvEntry.uvs[2].u = stream.ReadByte();
        uvEntry.uvs[2].v = stream.ReadByte();

        AddUV(&uvEntry);
    }
}
