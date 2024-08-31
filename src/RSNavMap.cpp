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
		name = stream.ReadString();
		read+=name.size()+1;
		if (read<9) {
			stream.MoveForward(9-read);
			read+=9-read;
		};
		int32_t shape_size = stream.ReadUInt32BE();
		read+=4;
		maps[name] = new RLEShape();
		maps[name]->Init(stream.ReadBytes(shape_size).data(), shape_size);
		read+=shape_size;
	}
}
void RSNavMap::parseNMAP_FONT(uint8_t *data, size_t size) {

}
void RSNavMap::parseNMAP_TEXT(uint8_t *data, size_t size) {

}
void RSNavMap::parseNMAP_SHAP(uint8_t* data, size_t size) {
	this->background = new RLEShape();
	this->background->Init(data, size);
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