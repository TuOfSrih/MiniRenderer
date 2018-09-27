// RenderingEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "vulkan\vulkan.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include <vector>

#define ASSERT_VK(val)\
	if(val != VK_SUCCESS){\
		std::cerr << val;\
		__debugbreak();\
	}

VkInstance instance;
VkSurfaceKHR surface;
VkDevice device;
VkSwapchainKHR swapchain;
uint32_t amountImagesInSwapchain = 0;
VkImageView *imageViews;
GLFWwindow *window;
VkShaderModule shaderModuleVert, shaderModuleFrag;
VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;
VkPipeline pipeline;

const uint32_t WIDTH = 1600;
const uint32_t HEIGHT = 900;
const std::vector<const char*> enabledValidationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};
std::vector<const char*> enabledExtensions = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};
const VkFormat usedFormat = VK_FORMAT_B8G8R8A8_UNORM;

VkDebugUtilsMessengerEXT callback;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void * pUserData) {
	if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		std::cerr << "Validationlayer: " << pCallbackData->pMessage << std::endl;
	}
	return VK_FALSE;
}
VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pCallback) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, 
	const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func) {
		func(instance, callback, pAllocator);
	}
}

void printStats(VkPhysicalDevice &device) {
	VkPhysicalDeviceProperties devProperties;
	vkGetPhysicalDeviceProperties(device, &devProperties);
	std::cout << "Name: " << devProperties.deviceName << std::endl << "API Version: " << VK_VERSION_MAJOR(devProperties.apiVersion) << "."
		<< VK_VERSION_MINOR(devProperties.apiVersion) << "." << VK_VERSION_PATCH(devProperties.apiVersion) << std::endl
		<< "Driver Version: " << devProperties.driverVersion << std::endl
		<< "Vendor ID:" << devProperties.vendorID << std::endl
		<< "Device Type: " << devProperties.deviceType << std::endl
		<< "discreteQueuePriorities: " << devProperties.limits.discreteQueuePriorities
		<< std::endl;

	VkPhysicalDeviceFeatures devFeatures;
	vkGetPhysicalDeviceFeatures(device, &devFeatures);

	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(device, &memProps);

	uint32_t amountQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &amountQueueFamilies, nullptr);
	VkQueueFamilyProperties *queueFamilyProperties = new VkQueueFamilyProperties[amountQueueFamilies];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &amountQueueFamilies, queueFamilyProperties);

	std::cout << "Amount of Queuefamilies: " << amountQueueFamilies << std::endl;

	for (unsigned int i = 0; i < amountQueueFamilies; i++) {
		uint32_t width = queueFamilyProperties[i].minImageTransferGranularity.width;
		uint32_t height = queueFamilyProperties[i].minImageTransferGranularity.height;
		uint32_t depth = queueFamilyProperties[i].minImageTransferGranularity.depth;
		std::cout << std::endl << "Queuefamily Nr: " << i << " VK_QUEUE_GRAPHICS_BIT       "
			<< ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			<< std::endl << "Queuefamily Nr: " << i <<       " VK_QUEUE_COMPUTE_BIT        "
			<< ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
			<< std::endl << "Queuefamily Nr: " << i <<       " VK_QUEUE_TRANSFER_BIT       "
			<< ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
			<< std::endl << "Queuefamily Nr: " << i <<       " VK_QUEUE_SPARSE_BINDING_BIT "
			<< ((queueFamilyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0) 
			<< std::endl << "Queuecount: " << queueFamilyProperties[i].queueCount
			<< std::endl << "Timestamp Valid Bits: " << queueFamilyProperties[i].timestampValidBits 
			<< std::endl << "Min Image Timestamp Granularity " << width << ", " << height << ", " << depth
			<< std::endl << std::endl;
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surfaceCapabilities);

	std::cout << "SurfaceCapabilities" << std::endl;
	std::cout << "\tminImageCount: " << surfaceCapabilities.minImageCount << std::endl;
	std::cout << "\tmaxImageCount: " << surfaceCapabilities.maxImageCount << std::endl;
	std::cout << "\tcurrentExtent: " << surfaceCapabilities.currentExtent.width << ":" << surfaceCapabilities.currentExtent.height << std::endl;
	std::cout << "\tminImageExtent: " << surfaceCapabilities.minImageExtent.width << ":" << surfaceCapabilities.minImageExtent.height << std::endl;
	std::cout << "\tmaxImageExtent: " << surfaceCapabilities.maxImageExtent.width << ":" << surfaceCapabilities.maxImageExtent.height << std::endl;
	std::cout << "\tmaxImageArrayLayers: " << surfaceCapabilities.maxImageArrayLayers << std::endl;
	std::cout << "\tsupportedTransforms:" << surfaceCapabilities.supportedTransforms << std::endl;
	std::cout << "\tcurrentTransform:" << surfaceCapabilities.currentTransform << std::endl;
	std::cout << "\tsupportedCompositeAlpha:" << surfaceCapabilities.supportedCompositeAlpha << std::endl;
	std::cout << "\tsupportedUsageFlags:" << surfaceCapabilities.supportedUsageFlags << std::endl;


	uint32_t amountFormats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &amountFormats, nullptr);
	VkSurfaceFormatKHR *surfaceFormats = new VkSurfaceFormatKHR[amountFormats];
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &amountFormats, surfaceFormats);

	std::cout << std::endl << "Amount of Formats: " << amountFormats << std::endl;
	for (int i = 0; i < amountFormats; i++) {
		std::cout << "Format: " << surfaceFormats[i].format << std::endl;
	}

	uint32_t amountPresentationmodes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &amountPresentationmodes, nullptr);
	VkPresentModeKHR *presentationsmodes = new VkPresentModeKHR[amountPresentationmodes];
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &amountPresentationmodes, presentationsmodes);

	std::cout << std::endl << "Amount of Presentmodes: " << amountPresentationmodes << std::endl;
	for (int i = 0; i < amountPresentationmodes; i++) {
		std::cout << "Presentmode: " << presentationsmodes[i] << std::endl;
	}

	std::cout << std::endl;
	delete[] surfaceFormats;
	delete[] queueFamilyProperties;
}

std::vector<char> readFile(const std::string &filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file) {
		std::cerr << "Could not open shader!" <<  std::endl;
		throw std::runtime_error("Could not open shaderfile!");
	}
	size_t size = (size_t) file.tellg();
	std::vector<char> fileBuffer(size);
	file.seekg(0);
	file.read(fileBuffer.data(), size);
	file.close();
	return fileBuffer;
}

void startGLFW() {
	glfwInit(); glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Tech Demo", nullptr, nullptr);
}

void createShaderModule(const std::vector<char> &code, VkShaderModule* shaderModule) {
	VkShaderModuleCreateInfo shaderModuleCreateInfo;
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = (uint32_t*)code.data();

	ASSERT_VK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, shaderModule));
}


void startVulkan() {
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "RenderingEngine";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName = "RenderingEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	uint32_t layerAmount = 0;
	vkEnumerateInstanceLayerProperties(&layerAmount, nullptr);
	VkLayerProperties *layers = new VkLayerProperties[layerAmount];
	vkEnumerateInstanceLayerProperties(&layerAmount, layers);

	std::cout << "Amount of layers: " << layerAmount << std::endl;
	for (int i = 0; i < layerAmount; i++) {
		std::cout << "Name:        " << layers[i].layerName << std::endl
			<< "Description: " << layers[i].description << std::endl << std::endl;
	}

	uint32_t extensionAmount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionAmount, nullptr);
	VkExtensionProperties *extensions = new VkExtensionProperties[extensionAmount];
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionAmount, extensions);

	std::cout << std::endl
		<< "Amount of Extension: " << extensionAmount << std::endl;
	for (int i = 0; i < extensionAmount; i++) {
		std::cout << "Name:        " << extensions[i].extensionName << std::endl;
	}


	//TODO: Improve adding
	uint32_t amountGLFWExtensions = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&amountGLFWExtensions);
	for (int i = 0; i < amountGLFWExtensions; i++) {
		enabledExtensions.push_back(glfwExtensions[i]);
	}



	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = enabledValidationLayers.size();
	instanceInfo.ppEnabledLayerNames = enabledValidationLayers.data();
	instanceInfo.enabledExtensionCount = enabledExtensions.size();
	instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
	std::cout << &instanceInfo << " " << &instance;
	ASSERT_VK(vkCreateInstance(&instanceInfo, nullptr, &instance));

	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo;
	debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCreateInfo.pNext = nullptr;
	debugUtilsMessengerCreateInfo.flags = 0;
	debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugUtilsMessengerCreateInfo.pfnUserCallback = debugCallback;
	debugUtilsMessengerCreateInfo.pUserData = nullptr;

	ASSERT_VK(createDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfo, nullptr, &callback ));


	ASSERT_VK(glfwCreateWindowSurface(instance, window, nullptr, &surface))

	vkGetInstanceProcAddr(instance, "");


	uint32_t numberPhysDevices = 0;
	ASSERT_VK(vkEnumeratePhysicalDevices(instance, &numberPhysDevices, nullptr));

	VkPhysicalDevice* physDevices = new VkPhysicalDevice[numberPhysDevices];

	ASSERT_VK(vkEnumeratePhysicalDevices(instance, &numberPhysDevices, physDevices));

	for (int i = 0; i < numberPhysDevices; i++) {
		printStats(physDevices[i]);
	}

	float queuePrios[] = { 1.0, 1.0, 1.0, 1.0 };

	VkDeviceQueueCreateInfo devQueueCreateInfo;
	devQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	devQueueCreateInfo.pNext = nullptr;
	devQueueCreateInfo.flags = 0;
	//TODO: Choose Familiy Index based on fitting properties
	devQueueCreateInfo.queueFamilyIndex = 0;
	devQueueCreateInfo.queueCount = 1;
	devQueueCreateInfo.pQueuePriorities = queuePrios;

	VkPhysicalDeviceFeatures enabledFeatures = {};
	VkDeviceCreateInfo devCreateInfo;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
//		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	devCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devCreateInfo.pNext = nullptr;
	devCreateInfo.flags = 0;
	devCreateInfo.queueCreateInfoCount = 1;
	devCreateInfo.pQueueCreateInfos = &devQueueCreateInfo;
	devCreateInfo.enabledLayerCount = 0;
	devCreateInfo.ppEnabledLayerNames = nullptr;
	devCreateInfo.enabledExtensionCount = deviceExtensions.size();
	devCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	devCreateInfo.pEnabledFeatures = &enabledFeatures;

	//TODO: Pick best suitable device instead of first one
	ASSERT_VK(vkCreateDevice(physDevices[0], &devCreateInfo, nullptr, &device));

	VkQueue dQueue;
	vkGetDeviceQueue(device, 0, 0, &dQueue);

	VkBool32 surfaceSupport = false;
	ASSERT_VK(vkGetPhysicalDeviceSurfaceSupportKHR(physDevices[0], 0, surface, &surfaceSupport));

	if (!surfaceSupport) {
		std::cerr << "Surface not supported!" << std::endl;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo;

	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 3;	//TODO: Check validity
	swapchainCreateInfo.imageFormat = usedFormat;
	swapchainCreateInfo.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent = VkExtent2D{ WIDTH, HEIGHT};
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	ASSERT_VK(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain));

	
	vkGetSwapchainImagesKHR(device, swapchain, &amountImagesInSwapchain, nullptr);
	VkImage *swapchainImages = new VkImage[amountImagesInSwapchain];
	ASSERT_VK(vkGetSwapchainImagesKHR(device, swapchain, &amountImagesInSwapchain, swapchainImages));

	imageViews = new VkImageView[amountImagesInSwapchain];
	for (int i = 0; i < amountImagesInSwapchain; i++) {
		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
		imageViewCreateInfo.components = VkComponentMapping{ VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		ASSERT_VK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]));
	}

	std::vector<char> shaderCodeVert = readFile("vert.spv");
	std::vector<char> shaderCodeFrag = readFile("frag.spv");

	

	createShaderModule(shaderCodeVert, &shaderModuleVert);
	createShaderModule(shaderCodeFrag, &shaderModuleFrag);

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfoVert;
	pipelineShaderStageCreateInfoVert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageCreateInfoVert.pNext = nullptr;
	pipelineShaderStageCreateInfoVert.flags = 0;
	pipelineShaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineShaderStageCreateInfoVert.module = shaderModuleVert;
	pipelineShaderStageCreateInfoVert.pName = "main";
	pipelineShaderStageCreateInfoVert.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfoFrag;
	pipelineShaderStageCreateInfoFrag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageCreateInfoFrag.pNext = nullptr;
	pipelineShaderStageCreateInfoFrag.flags = 0;
	pipelineShaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipelineShaderStageCreateInfoFrag.module = shaderModuleVert;
	pipelineShaderStageCreateInfoFrag.pName = "main";
	pipelineShaderStageCreateInfoFrag.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo stages[] = { pipelineShaderStageCreateInfoVert, pipelineShaderStageCreateInfoFrag };

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.pNext = nullptr;
	vertexInputStateCreateInfo.flags = 0;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo;
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.pNext = nullptr;
	pipelineInputAssemblyStateCreateInfo.flags = 0;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewPort;
	viewPort.x = 0.0f;
	viewPort.y = 0.0f;
	viewPort.width = WIDTH;
	viewPort.height = HEIGHT;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = { WIDTH, HEIGHT };

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo;
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.pNext = nullptr;
	pipelineViewportStateCreateInfo.flags = 0;
	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.pViewports =	&viewPort;
	pipelineViewportStateCreateInfo.scissorCount = 1;
	pipelineViewportStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo;
	pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.pNext = nullptr;
	pipelineRasterizationStateCreateInfo.flags = 0;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
	pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
	pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo;
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.pNext = nullptr;
	pipelineMultisampleStateCreateInfo.flags = 0;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f;
	pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
	pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState;
	pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
	pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo;
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.pNext = nullptr;
	pipelineColorBlendStateCreateInfo.flags = 0;
	pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	pipelineColorBlendStateCreateInfo.attachmentCount = 1;
	pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;
	pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	
	ASSERT_VK(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	VkAttachmentDescription attachmentDescription;
	attachmentDescription.flags = 0;
	attachmentDescription.format = usedFormat;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference attachmentReference;
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &attachmentReference;
	subpassDescription.pResolveAttachments = nullptr;
	subpassDescription.pDepthStencilAttachment = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo renderPassCreateInfo;
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = nullptr;

	ASSERT_VK(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass));

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = stages;
	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	pipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;
	
	std::cout << &pipelineCreateInfo << ", " << &pipeline;
	//ASSERT_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline));



	delete[] swapchainImages;
	delete[] layers;
	delete[] extensions;
	delete[] physDevices;

}

void gameloop() {

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void shutdownGLFW() {


	glfwDestroyWindow(window);
}

void shutdownVulkan() {
	vkDeviceWaitIdle(device);

	vkDestroyPipeline(device, pipeline, nullptr); 
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

	for (int i = 0; i < amountImagesInSwapchain; i++) {
		vkDestroyImageView(device, imageViews[i], nullptr);
	}
	delete[] imageViews;

	vkDestroyShaderModule(device, shaderModuleVert, nullptr);
	vkDestroyShaderModule(device, shaderModuleFrag, nullptr);

	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	destroyDebugUtilsMessengerEXT(instance, callback, nullptr);
	vkDestroyInstance(instance, nullptr);
}



int main() {

	startGLFW();
	startVulkan();
	
	gameloop();
	shutdownVulkan();
	shutdownGLFW();


	return 0;
}

