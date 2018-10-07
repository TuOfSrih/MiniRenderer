#pragma once



#define ASSERT_VK(val)\
	if(val != VK_SUCCESS){\
		std::cerr << val;\
		__debugbreak();\
	}


uint32_t findMemoryTypeIndex(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags memoryPropertyFlags);

void createBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize deviceSize, VkBufferUsageFlags bufferUsageFlags,
	VkBuffer &buffer, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory &deviceMemory);

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

	//copy
	copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}
