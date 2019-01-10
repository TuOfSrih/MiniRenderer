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

const std::vector<const char*>& Settings::getValidationLayers() {

	return active->fixed.enabledValidationLayers;
}

std::vector<const char*>& Settings::getInstanceExtensions() {

	return active->fixed.instanceExtensions;
}

const std::vector<const char*>&	Settings::getDeviceExtensions() {

	return active->fixed.deviceExtensions;
}

VkInstance& Settings::getInstance() {

	return active->fixed.instance;
}

VkDevice& Settings::getDevice() {

	return active->fixed.renderingDevice;
}

VkPhysicalDevice& Settings::getPhysDevice() {

	return active->fixed.physDevice;
}

VkQueue& Settings::getGraphicsQueue() {

	return Settings::active->fixed.graphicsQueue;
}

VkQueue& Settings::getPresentQueue() {

	return Settings::active->fixed.presentQueue;
}

VkQueue& Settings::getTransferQueue() {

	return Settings::active->fixed.transferQueue;
}

VkSurfaceKHR& Settings::getSurface() {

	return active->fixed.surface;
}

VkPresentModeKHR& Settings::getPresentMode() {

	return active->fixed.presentMode;
}

VkSurfaceFormatKHR&	Settings::getSurfaceFormat() {

	return active->fixed.surfaceFormat;
}

VkExtent2D& Settings::getSwapchainExtent() {

	return active->fixed.swapchainExtent;
}

uint32_t& Settings::getAmountImagesInSwapchain() {

	return active->fixed.amountImagesInSwapchain;
}

uint32_t& Settings::getGraphicsFamilyIndex() {

	return active->fixed.graphicsFamilyIndex;
}

uint32_t& Settings::getPresentFamilyIndex() {

	return active->fixed.presentFamilyIndex;
}

uint32_t& Settings::getTransferFamilyIndex() {

	return active->fixed.transferFamilyIndex;
}

uint8_t& Settings::getMaxFramesInFlight() {

	return active->fixed.MAX_FRAMES_IN_FLIGHT;
}

VkCommandPool& Settings::getCommandPool() {

	return active->fixed.commandPool;
}

VkDescriptorPool& Settings::getDescriptorPool() {

	return active->fixed.descriptorPool;
}

VkSwapchainKHR& Settings::getSwapchain() {

	return active->flex.swapchain;
}

uint32_t& Settings::getScreenWidth() {

	return active->flex.screenWidth;
}

uint32_t& Settings::getScreenHeight() {

	return active->flex.screenHeight;
}

