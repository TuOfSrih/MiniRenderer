#include "stdafx.h"
#include "Settings.h"

#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <string.h>

Settings* Settings::active;

enum StringValue {
	SHADER_DIR,
	UNDEFINED
};

void Settings::init() {

	Settings::active = new Settings();
}

void Settings::destroy() {

	delete Settings::active;
}

Settings::Settings(){

	Fixed fixedSettings;
	std::ifstream inStream("Config/Settings.cfg");

	//Parsing config file line by line
	std::string curLine;
	while (std::getline(inStream, curLine)) {

		std::stringstream lineStream(curLine);

		//Skip comments
		std::string delims(" \t");
		if (curLine.length() > 0 && curLine.find_first_not_of(delims) == '#') {
			
			continue;
		}

		//read option into respective value
		std::string option, value;
		lineStream >> option >> value;
		//TODO Check no more arguments

		//TODO Solution that scales better
		if (option == "ShaderDir") {

			fixed.shaderDir = std::move(value);
			continue;
		}

	}

}


std::string& Settings::getShaderDir() {

	return active->fixed.shaderDir;
}

VkDevice& Settings::getDevice() {

	return active->flex.renderingDevice;
}

VkPhysicalDevice& Settings::getPhysDevice() {

	return active->flex.physDevice;
}



