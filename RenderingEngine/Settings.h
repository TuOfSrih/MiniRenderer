#pragma once

struct Fixed {

	//General settings
	std::string shaderDir;

	//Graphics settings
	VkInstance			instance;
	VkDevice			renderingDevice;
	VkPhysicalDevice	physDevice;
	VkSurfaceKHR		surface;

	VkPresentModeKHR	presentMode;
	uint32_t			graphicsFamilyIndex;
	uint32_t			presentFamilyIndex;
	uint32_t			transferFamilyIndex;
	

	uint8_t				MAX_FRAMES_IN_FLIGHT = 2;
};

struct Flexible {

	//Graphics settings
	VkSwapchainKHR		swapchain		= VK_NULL_HANDLE;

	uint32_t			screenWidth;
	uint32_t			screenHeight;
	
};

class Settings {

private:

	Fixed		fixed;
	Flexible	flex;

	static Settings* active;

public:

	static void init();
	static void destroy();
	

	Settings();
	~Settings()									= default;
	Settings(const Settings& other)				= delete;
	Settings(Settings&& other)					= default;
	Settings& operator=(const Settings& other)	= delete;
	Settings& operator=(Settings&& other)		= default;


	static std::string&			getShaderDir();

	static VkInstance&			getInstance();
	static VkDevice&			getDevice();
	static VkPhysicalDevice&	getPhysDevice();
	static VkSurfaceKHR&		getSurface();

	static VkPresentModeKHR&	getPresentMode();
	static uint32_t&			getGraphicsFamilyIndex();
	static uint32_t&			getPresentFamilyIndex();
	static uint32_t&			getTransferFamilyIndex();

	static uint8_t&				getMaxFramesInFlight();


	static VkSwapchainKHR&		getSwapchain();
	static uint32_t&			getScreenWidth();
	static uint32_t&			getScreenHeight();
};

