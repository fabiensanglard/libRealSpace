#include "RSNavMap.h"

void RSNavMap::parseNMAP(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["MAPS"] = std::bind(&RSNavMap::parseNMAP_MAPS, this, std::placeholders::_1, std::placeholders::_2);
	handlers["FONT"] = std::bind(&RSNavMap::parseNMAP_FONT, this, std::placeholders::_1, std::placeholders::_2);
	handlers["TEXT"] = std::bind(&RSNavMap::parseNMAP_TEXT, this, std::placeholders::_1, std::placeholders::_2);
	handlers["SHAP"] = std::bind(&RSNavMap::parseNMAP_SHAP, this, std::placeholders::_1, std::placeholders::_2);
	
    lexer.InitFromRAM(data, size, handlers);
}
void RSNavMap::parseNMAP_MAPS(uint8_t *data, size_t size) {
    ByteStream stream(data);
	std::string name;
	size_t read=0;
	while (read < size) {
		name = stream.ReadString(8);
		read+=8;
		uint32_t shape_size = stream.ReadUInt32LE();
		read+=4;
		uint8_t *shape_data = (uint8_t*) malloc(shape_size);
		memcpy(shape_data, stream.GetPosition()+4, shape_size);
		maps[name] = new RLEShape();
		maps[name]->Init(shape_data, 0);
		stream.MoveForward(shape_size-4);
		read+=(shape_size-4);
	}
}
void RSNavMap::parseNMAP_FONT(uint8_t *data, size_t size) {

}
void RSNavMap::parseNMAP_TEXT(uint8_t *data, size_t size) {

}
void RSNavMap::parseNMAP_SHAP(uint8_t* data, size_t size) {
	ByteStream stream(data);
	this->background = new RLEShape();
	uint8_t *shape_data;
	shape_data = (uint8_t*) malloc(size);
	memcpy(shape_data, stream.GetPosition()+8, size);
	this->background->Init(shape_data, 0);
}

RSNavMap::RSNavMap() {

}
RSNavMap::~RSNavMap() {

}
void RSNavMap::InitFromRam(uint8_t *data, size_t size) {
    IFFSaxLexer lexer;

	std::map<std::string, std::function<void(uint8_t* data, size_t size)>> handlers;
	handlers["NMAP"] = std::bind(&RSNavMap::parseNMAP, this, std::placeholders::_1, std::placeholders::_2);
	lexer.InitFromRAM(data, size, handlers);
}