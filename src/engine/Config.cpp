#include "Config.hpp"

bool Config::load(const std::string& filePath) {
    _ini.SetUnicode();
    SI_Error result = _ini.LoadFile(filePath.c_str());
    return result == SI_OK;
}

std::string Config::getString(const std::string& section, const std::string& key, const std::string& defaultValue) const {
    return _ini.GetValue(section.c_str(), key.c_str(), defaultValue.c_str());
}

void Config::setString(const std::string &section, const std::string &key, const std::string &value) {
    _ini.SetValue(section.c_str(), key.c_str(), value.c_str());
}

int Config::getInt(const std::string& section, const std::string& key, int defaultValue) const {
    return static_cast<int>(_ini.GetLongValue(section.c_str(), key.c_str(), defaultValue));
}
int Config::setInt(const std::string& section, const std::string& key, int value) {
    _ini.SetLongValue(section.c_str(), key.c_str(), value);
    return value;
}
bool Config::getBool(const std::string& section, const std::string& key, bool defaultValue) const {
    return _ini.GetBoolValue(section.c_str(), key.c_str(), defaultValue);
}

void Config::setBool(const std::string &section, const std::string &key, bool value) {
    _ini.SetBoolValue(section.c_str(), key.c_str(), value);
}

void Config::save(const std::string &filePath) const {
    _ini.SaveFile(filePath.c_str());
}
