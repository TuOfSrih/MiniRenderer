#include "stdafx.h"
#include "Settings.h"

#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <string.h>

enum StringValue {
	SHADER_DIR,
	UNDEFINED
};

constexpr StringValue toValue(const char s[]) {

	strcmp(s, "ShaderDirectory") != 0 ? SHADER_DIR:
		UNDEFINED;
}

Settings::Settings()
{
	Fixed fixedSettings;
	std::ifstream inStream("Config/Settings.cfg");

	std::string curLine;
	while (std::getline(inStream, curLine)) {

		std::stringstream lineStream(curLine);

		//Skip comments
		std::string delims(" \t");
		if (curLine.length() > 0 && curLine.find_first_not_of(delims) == '#') {
			
			continue;
		}

		std::string option, value;
		lineStream >> option >> value;
		//TODO Check no more arguments

		//TODO Solution that scales better
		if (option == "ShaderDir") {

			fixed.shaderDir = value;
			continue;
		}

	}

}


Settings::~Settings()
{
}

std::string& Settings::getShaderDir() {

	return fixed.shaderDir;
}
