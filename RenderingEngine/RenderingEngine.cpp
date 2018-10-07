// RenderingEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Utils.h"
//#define STB_IMAGE_IMPLEMENTATION
#include "ImageHolder.h"


#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <chrono>

#include <iostream>
#include <fstream>
#include <vector>



VkQueue dQueue;
VkInstance instance;
VkDebugUtilsMessengerEXT callback;
VkSurfaceKHR surface;
VkPhysicalDevice* physDevices;
VkDevice device;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;
uint32_t amountImagesInSwapchain = 0;
VkImageView *imageViews;
GLFWwindow *window;
VkShaderModule shaderModuleVert, shaderModuleFrag;
VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;
VkPipeline pipeline;
VkFramebuffer *framebuffers;
VkCommandPool commandPool;
VkCommandBuffer *commandBuffers;
VkSemaphore semaphoreImageAvailable;
VkSemaphore semaphoreRenderingFinished;
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferDeviceMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferDeviceMemory;

VkBuffer matrixBuffer;
VkDeviceMemory matrixBufferDeviceMemory;

VkDescriptorSetLayout descriptorSetLayout;
VkDescriptorPool descriptorPool;
VkDescriptorSet descriptorSet;

uint32_t screenWidth = 1600;
uint32_t screenHeight = 900;
const std::vector<const char*> enabledValidationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};
std::vector<const char*> enabledExtensions = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME
};

ImageHolder* debugTexture;	//TODO hunt down bug with different instances

const VkFormat usedFormat = VK_FORMAT_B8G8R8A8_UNORM;

auto gameStartTime = std::chrono::high_resolution_clock::now();

glm::mat4 modelViewProjection;

class Vertex {
	public:
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getVertexInputBindingDescription() {
		VkVertexInputBindingDescription vertexInputBindingDescription;
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(Vertex);
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return vertexInputBindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributDescriptions() {
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributDescriptions(2);
		
		vertexInputAttributDescriptions[0].location = 0;
		vertexInputAttributDescriptions[0].binding = 0;
		vertexInputAttributDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributDescriptions[0].offset = offsetof(Vertex, pos);

		vertexInputAttributDescriptions[1].location = 1;
		vertexInputAttributDescriptions[1].binding = 0;
		vertexInputAttributDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributDescriptions[1].offset = offsetof(Vertex, color);

		return vertexInputAttributDescriptions;
	}
};

std::vector<Vertex> vertices = {
	{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f   } },
	{ {-0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f   } },
	{ { 0.5f, -0.5f },{ 1.0f, 1.0f, 0.0f  } }
};

std::vector<uint32_t> indices = {
	0, 1, 2,
	0, 3, 1
};


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void * pUserData) {
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
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

void recreateSwapchain();
void onWindowResized(GLFWwindow* window, int width, int height) {
	if (width == 0 || height == 0) return; // Nothing to Do
	screenWidth = width;
	screenHeight = height;
	recreateSwapchain();
}

void startGLFW() {
	glfwInit(); 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(screenWidth, screenHeight, "Tech Demo", nullptr, nullptr);
	glfwSetWindowSizeCallback(window, onWindowResized);
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

void createInstance() {
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "RenderingEngine";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName = "RenderingEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

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
}

void printValidationLayers() {
	uint32_t layerAmount = 0;
	vkEnumerateInstanceLayerProperties(&layerAmount, nullptr);
	VkLayerProperties *layers = new VkLayerProperties[layerAmount];
	vkEnumerateInstanceLayerProperties(&layerAmount, layers);

	std::cout << "Amount of layers: " << layerAmount << std::endl;
	for (int i = 0; i < layerAmount; i++) {
		std::cout << "Name:        " << layers[i].layerName << std::endl
			<< "Description: " << layers[i].description << std::endl << std::endl;
	}

	delete[] layers;
}

void printExtensions() {
	uint32_t extensionAmount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionAmount, nullptr);
	VkExtensionProperties *extensions = new VkExtensionProperties[extensionAmount];
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionAmount, extensions);

	std::cout << std::endl
		<< "Amount of Extension: " << extensionAmount << std::endl;
	for (int i = 0; i < extensionAmount; i++) {
		std::cout << "Name:        " << extensions[i].extensionName << std::endl;
	}

	delete[] extensions;
}

void setUpDebugCallbacks() {
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

	ASSERT_VK(createDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfo, nullptr, &callback));
}

void printStatsPhysicalDevices() {
	uint32_t numberPhysDevices = 0;
	ASSERT_VK(vkEnumeratePhysicalDevices(instance, &numberPhysDevices, nullptr));

	physDevices = new VkPhysicalDevice[numberPhysDevices];

	ASSERT_VK(vkEnumeratePhysicalDevices(instance, &numberPhysDevices, physDevices));

	for (int i = 0; i < numberPhysDevices; i++) {
		printStats(physDevices[i]);
	}
}

void createDevice() {
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
}

void checkSurfaceSupport() {
	VkBool32 surfaceSupport = false;
	ASSERT_VK(vkGetPhysicalDeviceSurfaceSupportKHR(physDevices[0], 0, surface, &surfaceSupport));

	if (!surfaceSupport) {
		std::cerr << "Surface not supported!" << std::endl;
	}
}

void createSwapchain() {
	VkSwapchainCreateInfoKHR swapchainCreateInfo;

	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 3;	//TODO: Check validity
	swapchainCreateInfo.imageFormat = usedFormat;
	swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent = VkExtent2D{ screenWidth, screenHeight };
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = swapchain;

	ASSERT_VK(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain));
}

void createImageViews() {
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
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		ASSERT_VK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]));
	}
	delete[] swapchainImages;
}

void createRenderPass() {
	

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

	VkSubpassDependency subpassDependency;
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = 0;

	VkRenderPassCreateInfo renderPassCreateInfo;
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	ASSERT_VK(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass));
}

void createDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
	descriptorSetLayoutBinding.binding = 0;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = nullptr;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

	ASSERT_VK(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));

}

void createPipeline() {
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
	pipelineShaderStageCreateInfoFrag.module = shaderModuleFrag;
	pipelineShaderStageCreateInfoFrag.pName = "main";
	pipelineShaderStageCreateInfoFrag.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[] = { pipelineShaderStageCreateInfoVert, pipelineShaderStageCreateInfoFrag };

	VkVertexInputBindingDescription vertexInputBindingDescription = Vertex::getVertexInputBindingDescription();
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescritions = Vertex::getVertexInputAttributDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.pNext = nullptr;
	vertexInputStateCreateInfo.flags = 0;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescritions.size();
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescritions.data();

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo;
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.pNext = nullptr;
	pipelineInputAssemblyStateCreateInfo.flags = 0;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewPort;
	viewPort.x = 0.0f;
	viewPort.y = 0.0f;
	viewPort.width = screenWidth;
	viewPort.height = screenHeight;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = { screenWidth, screenHeight };

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo;
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.pNext = nullptr;
	pipelineViewportStateCreateInfo.flags = 0;
	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.pViewports = &viewPort;
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
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo;
	pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.pNext = nullptr;
	pipelineDynamicStateCreateInfo.flags;
	pipelineDynamicStateCreateInfo.dynamicStateCount = sizeof(dynamicStates)/sizeof(*dynamicStates);
	pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	ASSERT_VK(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	pipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	pipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	ASSERT_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline));
}

void createFrameBuffers() {
	framebuffers = new VkFramebuffer[amountImagesInSwapchain];
	for (size_t i = 0; i < amountImagesInSwapchain; i++) {
		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &imageViews[i];
		framebufferCreateInfo.width = screenWidth;
		framebufferCreateInfo.height = screenHeight;
		framebufferCreateInfo.layers = 1;

		ASSERT_VK(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]));
	}
}

void createCommandPool() {
	VkCommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = 0;// Check validiy with queue having graphics bit

	ASSERT_VK(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool));
}

void allocateCommandBuffers() {
	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = amountImagesInSwapchain;

	commandBuffers = new VkCommandBuffer[amountImagesInSwapchain];
	ASSERT_VK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers));
}




void copyBuffer(VkBuffer src, VkBuffer dest, VkDeviceSize deviceSize) {

	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	ASSERT_VK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	ASSERT_VK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

	VkBufferCopy bufferCopy;
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = deviceSize;

	vkCmdCopyBuffer(commandBuffer, src, dest, 1, &bufferCopy);

	ASSERT_VK(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	ASSERT_VK(vkQueueSubmit(dQueue, 1, &submitInfo, VK_NULL_HANDLE));// get Queue with only transfer bit

	vkQueueWaitIdle(dQueue);

	//optimisation with fences

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

}

void loadTexture() {
	debugTexture = new ImageHolder("resources/debug.png");
	//ImageHolder debugTexture();
	std::cout << debugTexture->getSize();

}

void createVertexBuffer() {

	createAndUploadBuffer<Vertex>(device, physDevices[0], dQueue, commandPool, vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferDeviceMemory);

}

void createIndexBuffer() {
	createAndUploadBuffer<uint32_t>(device, physDevices[0], dQueue, commandPool, indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferDeviceMemory);
}

void createMatrixBuffer() {
	VkDeviceSize deviceSize = sizeof(modelViewProjection);
	createBuffer(device, physDevices[0], deviceSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, matrixBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, matrixBufferDeviceMemory);
}

void createDescriptorPool() {
	VkDescriptorPoolSize descriptorPoolSize;
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

	ASSERT_VK(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));
}

void allocateDescriptorSet() {
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = nullptr;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

	ASSERT_VK(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

	VkDescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = matrixBuffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(modelViewProjection);

	VkWriteDescriptorSet writeDescriptorSet;
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = nullptr;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.pImageInfo = nullptr;
	writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
	writeDescriptorSet.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void recordCommands() {
	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	for (size_t i = 0; i < amountImagesInSwapchain; i++) {
		ASSERT_VK(vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo));

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffers[i];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = { screenWidth, screenHeight };
		VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport viewPort;
		viewPort.x = 0.0f;
		viewPort.y = 0.0f;
		viewPort.width = screenWidth;
		viewPort.height = screenHeight;
		viewPort.minDepth = 0.0f;
		viewPort.maxDepth = 1.0f;

		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewPort);

		VkRect2D scissor;
		scissor.offset = { 0, 0};
		scissor.extent = { screenWidth, screenHeight };

		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);


		vkCmdDrawIndexed(commandBuffers[i], indices.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		ASSERT_VK(vkEndCommandBuffer(commandBuffers[i]));
	}
}

void createSemaphores() {
	VkSemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = 0;

	ASSERT_VK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphoreImageAvailable));
	ASSERT_VK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphoreRenderingFinished));
}

void recreateSwapchain() {

	vkDeviceWaitIdle(device);

	vkDeviceWaitIdle(device);

	vkFreeCommandBuffers(device, commandPool, amountImagesInSwapchain, commandBuffers);
	delete[] commandBuffers;
	vkDestroyCommandPool(device, commandPool, nullptr);
	for (size_t i = 0; i < amountImagesInSwapchain; i++) {
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}
	delete[] framebuffers;
	//vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	//vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

	for (int i = 0; i < amountImagesInSwapchain; i++) {
		vkDestroyImageView(device, imageViews[i], nullptr);
	}
	delete[] imageViews;

	//vkDestroyShaderModule(device, shaderModuleVert, nullptr);
	//vkDestroyShaderModule(device, shaderModuleFrag, nullptr);

	VkSwapchainKHR oldSwapchain = swapchain;

	createSwapchain();
	createImageViews();
	createRenderPass();
	//createPipeline();
	createFrameBuffers();
	createCommandPool();
	allocateCommandBuffers();
	recordCommands();

	vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
}

void startVulkan() {

	printValidationLayers();
	printExtensions();
	//TODO: Improve adding
	uint32_t amountGLFWExtensions = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&amountGLFWExtensions);
	for (int i = 0; i < amountGLFWExtensions; i++) {
		enabledExtensions.push_back(glfwExtensions[i]);
	}
	createInstance();
	setUpDebugCallbacks();
	ASSERT_VK(glfwCreateWindowSurface(instance, window, nullptr, &surface))
	printStatsPhysicalDevices();
	createDevice();
	vkGetDeviceQueue(device, 0, 0, &dQueue);
	checkSurfaceSupport();
	createSwapchain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createPipeline();
	createFrameBuffers();
	createCommandPool();
	allocateCommandBuffers();
	loadTexture();
	createVertexBuffer();
	createIndexBuffer();
	createMatrixBuffer();
	createDescriptorPool();
	allocateDescriptorSet();
	recordCommands();
	createSemaphores();
}

void drawFrame() {
	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), semaphoreImageAvailable, VK_NULL_HANDLE, &imageIndex);

	VkPipelineStageFlags waitStageMasks[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &semaphoreImageAvailable;
	submitInfo.pWaitDstStageMask = waitStageMasks;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphoreRenderingFinished;

	ASSERT_VK(vkQueueSubmit(dQueue, 1, &submitInfo, VK_NULL_HANDLE));

	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphoreRenderingFinished;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	ASSERT_VK(vkQueuePresentKHR(dQueue, &presentInfo));
}

void printMatrix(glm::mat4 matrix) {
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			std::cout << matrix[i][j] << ", ";
		}
		std::cout << std::endl;
	}
}

void updateMVP() {
	auto frameTime = std::chrono::high_resolution_clock::now();

	float timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(frameTime - gameStartTime).count() / 1000.0f;
	// TODO Improvement?
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), timeSinceStart * glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 projection = glm::perspective(glm::radians(60.0f),  screenWidth / (float) screenHeight, 0.01f, 10.0f);
	projection[1][1] *= -1;

	modelViewProjection = projection * view * model;
	

	void* data;
	vkMapMemory(device, matrixBufferDeviceMemory, 0, sizeof(modelViewProjection), 0, &data);
	memcpy(data, &modelViewProjection, sizeof(modelViewProjection));
	vkUnmapMemory(device, matrixBufferDeviceMemory);
}

void gameloop() {

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		updateMVP();
		drawFrame();
	}
}

void shutdownGLFW() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void shutdownVulkan() {
	vkDeviceWaitIdle(device);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkFreeMemory(device, matrixBufferDeviceMemory, nullptr);
	vkDestroyBuffer(device, matrixBuffer, nullptr);

	vkDestroySemaphore(device, semaphoreImageAvailable, nullptr);
	vkDestroySemaphore(device, semaphoreRenderingFinished, nullptr);

	vkFreeMemory(device, indexBufferDeviceMemory, nullptr);
	vkDestroyBuffer(device, indexBuffer, nullptr);

	vkFreeMemory(device, vertexBufferDeviceMemory, nullptr);
	vkDestroyBuffer(device, vertexBuffer, nullptr);

	//debugTexture->destroy();

	vkFreeCommandBuffers(device, commandPool, amountImagesInSwapchain, commandBuffers);
	delete[] commandBuffers;
	vkDestroyCommandPool(device, commandPool, nullptr);
	for (size_t i = 0; i < amountImagesInSwapchain; i++) {
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}
	delete[] framebuffers;
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
	delete[] physDevices;
	vkDestroySurfaceKHR(instance, surface, nullptr);
	destroyDebugUtilsMessengerEXT(instance, callback, nullptr);
	vkDestroyInstance(instance, nullptr);
}



int main() {

	std::ifstream testFile;
	testFile.open("resources/test.txt");
	if (!testFile) std::cerr << "Can't open input file!";
	startGLFW();
	startVulkan();
	
	gameloop();

	shutdownVulkan();
	shutdownGLFW();


	return 0;
}

