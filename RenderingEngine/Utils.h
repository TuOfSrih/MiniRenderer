#pragma once

#include <fstream>

#define ASSERT_VK(val)\
	if(val != VK_SUCCESS){\
		std::cerr << val;\
		__debugbreak();\
	}

struct Buffer {

	VkBuffer buffer;
	VkDeviceMemory memory;
	std::size_t size;
};


void notSupported(const char* msg);

uint32_t findMemoryTypeIndex(const uint32_t typeFilter, const VkMemoryPropertyFlags memoryPropertyFlags);

bool isFormatSupported(const VkFormat format, const VkImageTiling imageTiling, const VkFormatFeatureFlags formatFeatureFlags);

VkFormat findSupportedFormat(const std::vector<VkFormat> formats, const VkImageTiling imageTiling, const VkFormatFeatureFlags formatFeatureFlags);

bool isStencilFormat(const VkFormat format);

void createBuffer(const VkDeviceSize deviceSize, const VkBufferUsageFlags bufferUsageFlags,
	VkBuffer &buffer, const VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory &deviceMemory);

void copyBuffer(const VkBuffer src, const VkBuffer dest, const VkDeviceSize deviceSize);

VkCommandBuffer startOneTimeCommandBuffer();

void endOneTimeCommandBuffer(const VkQueue queue, const VkCommandPool commandPool, const VkCommandBuffer commandBuffer);

//TODO solution with less arguments
void createImage( const uint32_t width, const uint32_t height, const VkFormat format, const VkImageTiling imageTiling,
	const VkImageUsageFlags imageUsageFlags, const VkMemoryPropertyFlags memoryPropertyFlags, VkImage &image, VkDeviceMemory &imageMemory);

VkImageView createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags imageAspectFlags);

void changeImageLayout(const VkQueue queue, const VkImage image, const VkFormat format, const VkImageLayout oldImageLayout, const VkImageLayout newImageLayout);

std::vector<char> readBytesFromFile(const std::string &filename);

void createShaderModule(const std::vector<char> &code, VkShaderModule* const shaderModule);

void loadShader(const std::string& filename, VkShaderModule*const shaderModule);

//TODO investigate if queue can be left out
template<typename T>
void createAndUploadBuffer(const std::vector<T>& data, const VkBufferUsageFlags bufferUsageFlags, VkBuffer& buffer, VkDeviceMemory& deviceMemory) {
	
	const VkDeviceSize bufferSize = sizeof(T) *data.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBufferMemory);

	void *rawData;
	ASSERT_VK(vkMapMemory(Settings::getDevice(), stagingBufferMemory, 0, bufferSize, 0, &rawData));
	memcpy(rawData, data.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(Settings::getDevice(), stagingBufferMemory);

	createBuffer(bufferSize, bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, deviceMemory);
	copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(Settings::getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(Settings::getDevice(), stagingBufferMemory, nullptr);
}
