#pragma once

#define ASSERT_VK(val)\
	if(val != VK_SUCCESS){\
		std::cerr << val;\
		__debugbreak();\
	}


void notSupported(const char* msg);

uint32_t findMemoryTypeIndex(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags memoryPropertyFlags);

bool isFormatSupported(VkPhysicalDevice physDevice, VkFormat format, VkImageTiling imageTiling, VkFormatFeatureFlags formatFeatureFlags);

VkFormat findSupportedFormat(VkPhysicalDevice physDevice, const std::vector<VkFormat> formats, VkImageTiling imageTiling, VkFormatFeatureFlags formatFeatureFlags);

bool isStencilFormat(VkFormat format);

void createBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize deviceSize, VkBufferUsageFlags bufferUsageFlags,
	VkBuffer &buffer, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory &deviceMemory);

void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer src, VkBuffer dest, VkDeviceSize deviceSize);

VkCommandBuffer startOneTimeCommandBuffer(VkDevice device, VkCommandPool commandPool);

void endOneTimeCommandBuffer(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);

void createImage(VkDevice device, VkPhysicalDevice physDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling imageTiling,
	VkImageUsageFlags imageUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkImage &image, VkDeviceMemory &imageMemory);

void createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags imageAspectFlags, VkImageView &imageView);

void changeImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);

template<typename T>
void createAndUploadBuffer(VkDevice device, VkPhysicalDevice physDevice, VkQueue queue, VkCommandPool commandPool,
	
	std::vector<T> data, VkBufferUsageFlags bufferUsageFlags, VkBuffer &buffer, VkDeviceMemory deviceMemory) {
	VkDeviceSize bufferSize = sizeof(T) *data.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(device, physDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBufferMemory);

	void *rawData;
	ASSERT_VK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &rawData));
	memcpy(rawData, data.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(device, physDevice, bufferSize, bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, deviceMemory);
	copyBuffer(device, commandPool, queue, stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}
