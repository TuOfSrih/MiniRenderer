#pragma once

struct Fixed {

	//General settings
	std::string shaderDir;

	const std::vector<const char*>	enabledValidationLayers = {
	"VK_LAYER_LUNARG_standard_validation",
	};
	std::vector<const char*>		instanceExtensions = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	};
	const std::vector<const char*>	deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	//Graphics settings
	VkInstance						instance;
	VkDevice						renderingDevice;
	VkPhysicalDevice				physDevice;
	VkQueue							graphicsQueue;
	VkQueue							presentQueue;
	VkQueue							transferQueue;
	VkSurfaceKHR					surface;

	VkPresentModeKHR				presentMode;
	VkSurfaceFormatKHR				surfaceFormat;
	VkExtent2D						swapchainExtent;
	uint32_t						amountImagesInSwapchain;
	uint32_t						graphicsFamilyIndex;
	uint32_t						presentFamilyIndex;
	uint32_t						transferFamilyIndex;
	

	uint8_t							MAX_FRAMES_IN_FLIGHT = 2;

	//Memory settings
	VkCommandPool					commandPool;
	VkDescriptorPool				descriptorPool;


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


	static std::string&						getShaderDir();
	static const std::vector<const char*>&	getValidationLayers();
	static std::vector<const char*>&		getInstanceExtensions();
	static const std::vector<const char*>&	getDeviceExtensions();

	static VkInstance&						getInstance();
	static VkDevice&						getDevice();
	static VkPhysicalDevice&				getPhysDevice();
	static VkQueue&							getGraphicsQueue();
	static VkQueue&							getPresentQueue();
	static VkQueue&							getTransferQueue();
	static VkSurfaceKHR&					getSurface();

	static VkPresentModeKHR&				getPresentMode();
	static VkSurfaceFormatKHR&				getSurfaceFormat();
	static VkExtent2D&						getSwapchainExtent();
	static uint32_t&						getAmountImagesInSwapchain();
	static uint32_t&						getGraphicsFamilyIndex();
	static uint32_t&						getPresentFamilyIndex();
	static uint32_t&						getTransferFamilyIndex();

	static uint8_t&							getMaxFramesInFlight();

	static VkCommandPool&					getCommandPool();
	static VkDescriptorPool&				getDescriptorPool();


	static VkSwapchainKHR&					getSwapchain();
	static uint32_t&						getScreenWidth();
	static uint32_t&						getScreenHeight();
};

