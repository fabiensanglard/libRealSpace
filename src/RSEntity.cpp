//
//  RSEntity.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RSEntity.h"
#include <cfloat>

void parsRNWY(IffChunk* chunk, const char* name) {
	
}

RSEntity::RSEntity() :
	prepared(false)
{

}

RSEntity::~RSEntity() {
	while (!images.empty()) {
		RSImage* image = images.back();
		images.pop_back();
		delete image;
	}
}



void RSEntity::ParseTXMP(IffChunk* chunk) {
	ByteStream stream(chunk->data);

	RSImage* image = new RSImage();

	char name[9];

	for (int i = 0; i < 8; i++)
		name[i] = stream.ReadByte();

	uint32_t width = stream.ReadShort();
	uint32_t height = stream.ReadShort();

	image->Create(name, width, height, 0);
	image->UpdateContent(stream.GetPosition());
	AddImage(image);
}

void RSEntity::ParseTXMS(IffChunk* chunk) {

	if (chunk == NULL)
		return;

	if (chunk->childs.size() < 1) {
		//That is not expected, at minimun we should have an INFO chunk.
		printf("Error: This TXMS doesn't even have an INFO chunk.");
		return;
	}


	ByteStream stream(chunk->childs[0]->data);
	uint32_t version = stream.ReadUInt32LE();
	printf("TXMS Version: %u.\n", version);

	for (int i = 1; i < chunk->childs.size(); i++) {
		IffChunk* maybeTXMS = chunk->childs[i];
		if (maybeTXMS->id == 'TXMP')
			ParseTXMP(maybeTXMS);
	}

}

void RSEntity::ParseVERT(IffChunk* chunk) {

	if (chunk == NULL)
		return;

	ByteStream stream(chunk->data);

	size_t numVertice = chunk->size / 12;

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

void RSEntity::ParseLVL(IffChunk* chunk) {

	if (chunk == NULL)
		return;

	ByteStream stream(chunk->data);

	Lod lod;

	lod.numTriangles = static_cast<uint16_t>((chunk->size - 4) / 2);

	lod.dist = stream.ReadUInt32LE();

	for (int i = 0; i < lod.numTriangles; i++) {
		lod.triangleIDs[i] = stream.ReadUShort();
	}

	AddLod(&lod);
}


void RSEntity::ParseVTRI(IffChunk* chunk) {

	if (chunk == NULL)
		return;

	size_t numTriangle = chunk->size / 8;
	ByteStream stream(chunk->data);

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

void RSEntity::ParseUVXY(IffChunk* chunk) {

	if (chunk == NULL)
		return;

	ByteStream stream(chunk->data);

	size_t numEntries = chunk->size / 8;

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


void RSEntity::InitFromRAM(uint8_t* data, size_t size) {

	IffLexer lexer;
	lexer.InitFromRAM(data, size);
	InitFromIFF(&lexer);
}

void RSEntity::InitFromIFF(IffLexer* lexer) {


	IffChunk* chunk;

	chunk = lexer->GetChunkByID('UVXY');
	ParseUVXY(chunk);

	chunk = lexer->GetChunkByID('VTRI');
	ParseVTRI(chunk);

	chunk = lexer->GetChunkByID('VERT');
	ParseVERT(chunk);

	chunk = lexer->GetChunkByID('TXMS');
	ParseTXMS(chunk);

	chunk = lexer->GetChunkByID('LVL0');
	if (chunk != NULL)
		ParseLVL(chunk);

	chunk = lexer->GetChunkByID('LVL1');
	if (chunk != NULL)
		ParseLVL(chunk);

	chunk = lexer->GetChunkByID('LVL2');
	if (chunk != NULL)
		ParseLVL(chunk);

	/*printChunkF(lexer->GetChunkByID('EXTE'), "EXTE");
	printChunkF(lexer->GetChunkByID('NONE'), "NONE");
	printChunkF(lexer->GetChunkByID('DAMG'), "DAMG");
	printChunkF(lexer->GetChunkByID('TRGT'), "TRGT");
	parsRNWY(lexer->GetChunkByID('RNWY'), "RNWY"); */
	CalcBoundingBox();
}


BoudingBox* RSEntity::GetBoudingBpx(void) {
	return &this->bb;
}


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


size_t RSEntity::NumImages(void) {
	return this->images.size();
}

size_t RSEntity::NumVertice(void) {
	return this->vertices.size();
}

size_t RSEntity::NumUVs(void) {
	return this->uvs.size();
}

size_t RSEntity::NumLods(void) {
	return this->lods.size();
}

size_t RSEntity::NumTriangles(void) {
	return this->triangles.size();
}

void RSEntity::AddImage(RSImage* image) {
	this->images.push_back(image);
}

void RSEntity::AddVertex(Point3D* vertex) {
	this->vertices.push_back(*vertex);
}

void RSEntity::AddUV(uvxyEntry* uv) {
	this->uvs.push_back(*uv);
}

void RSEntity::AddLod(Lod* lod) {
	this->lods.push_back(*lod);
}

void RSEntity::AddTriangle(Triangle* triangle) {
	this->triangles.push_back(*triangle);
}

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
    handlers["JETP"] = std::bind(&RSEntity::parseREAL_OBJT_JETP, this, std::placeholders::_1, std::placeholders::_2);
	handlers["EXTE"] = std::bind(&RSEntity::parseREAL_OBJT_EXTE, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_INFO(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP(uint8_t *data, size_t size) {
	IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["EXPL"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_EXPL, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DEBR"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DEBR, this, std::placeholders::_1, std::placeholders::_2);
	handlers["DEST"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DEST, this, std::placeholders::_1, std::placeholders::_2);
	handlers["SMOK"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_SMOK, this, std::placeholders::_1, std::placeholders::_2);

	handlers["CHLD"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_CHLD, this, std::placeholders::_1, std::placeholders::_2);
    handlers["JINF"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_JINF, this, std::placeholders::_1, std::placeholders::_2);
	handlers["DAMG"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DAMG, this, std::placeholders::_1, std::placeholders::_2);
	handlers["EJEC"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_EJEC, this, std::placeholders::_1, std::placeholders::_2);

	handlers["SIGN"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_SIGN, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TRGT"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_TRGT, this, std::placeholders::_1, std::placeholders::_2);
	handlers["CTRL"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_CTRL, this, std::placeholders::_1, std::placeholders::_2);
	handlers["TOFF"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_TOFF, this, std::placeholders::_1, std::placeholders::_2);

	handlers["LAND"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_LAND, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DYNM"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM, this, std::placeholders::_1, std::placeholders::_2);
	handlers["WEAP"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP, this, std::placeholders::_1, std::placeholders::_2);
	handlers["DAMG"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DAMG, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_JETP_EXPL(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DEBR(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DEST(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_SMOK(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_CHLD(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_JINF(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DAMG(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_EJEC(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_SIGN(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_TRGT(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_CTRL(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_TOFF(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_LAND(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM(uint8_t *data, size_t size) {
	IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["DYNM"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_DYNM, this, std::placeholders::_1, std::placeholders::_2);
    handlers["ORDY"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_ORDY, this, std::placeholders::_1, std::placeholders::_2);
	handlers["STBL"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_STBL, this, std::placeholders::_1, std::placeholders::_2);
	handlers["ATMO"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_ATMO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["GRAV"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_GRAV, this, std::placeholders::_1, std::placeholders::_2);
	handlers["THRS"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_THRS, this, std::placeholders::_1, std::placeholders::_2);
	handlers["JDYN"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_DYNM_JDYN, this, std::placeholders::_1, std::placeholders::_2);

	lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_JETP_DYNM_DYNM(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM_ORDY(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM_STBL(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM_ATMO(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM_GRAV(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM_THRS(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DYNM_JDYN(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_WEAP(uint8_t *data, size_t size) {
	IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DCOY"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP_DCOY, this, std::placeholders::_1, std::placeholders::_2);
	handlers["WPNS"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP_WPNS, this, std::placeholders::_1, std::placeholders::_2);
	handlers["HPTS"] = std::bind(&RSEntity::parseREAL_OBJT_JETP_WEAP_HPTS, this, std::placeholders::_1, std::placeholders::_2);
    
	lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_JETP_WEAP_INFO(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_WEAP_DCOY(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_WEAP_WPNS(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_WEAP_HPTS(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_OBJT_JETP_DAMG(uint8_t *data, size_t size) {
	IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    
	lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_OBJT_EXTE(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_APPR(uint8_t *data, size_t size) {
	IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["POLY"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    
	lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_APPR_POLY(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_APPR_POLY_(uint8_t *data, size_t size) {
	IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["INFO"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    handlers["VERT"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    handlers["DETA"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TRIS"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    
	lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_APPR_POLY_INFO(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_APPR_POLY_VERT(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_APPR_POLY_DETA(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_APPR_POLY_TRIS(uint8_t *data, size_t size) {
	IFFSaxLexer lexer;

    std::map<std::string, std::function<void(uint8_t * data, size_t size)>> handlers;
    handlers["VTRI"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TXMS"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    handlers["UVXY"] = std::bind(&RSEntity::parseREAL_APPR_POLY, this, std::placeholders::_1, std::placeholders::_2);
    
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
    handlers["INFO"] = std::bind(&RSEntity::parseREAL_APPR_POLY_TRIS_TXMS_INFO, this, std::placeholders::_1, std::placeholders::_2);
    handlers["TXMP"] = std::bind(&RSEntity::parseREAL_APPR_POLY_TRIS_TXMS_TXMP, this, std::placeholders::_1, std::placeholders::_2);
   
	lexer.InitFromRAM(data, size, handlers);
}
void RSEntity::parseREAL_APPR_POLY_TRIS_TXMS_INFO(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_APPR_POLY_TRIS_TXMS_TXMP(uint8_t *data, size_t size) {}
void RSEntity::parseREAL_APPR_POLY_TRIS_UVXY(uint8_t *data, size_t size) {
	if (data == NULL)
		return;

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
