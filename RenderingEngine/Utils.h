#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include <vector>
#include <iostream>

#define ASSERT_VK(val)\
	if(val != VK_SUCCESS){\
		std::cerr << val;\
		__debugbreak();\
	}


uint32_t findMemoryTypeIndex(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags memoryPropertyFlags) {
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physDevice, &physicalDeviceMemoryProperties);//Check validity
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)) {//Research on memory types
			return i;
		}
	}

	throw std::runtime_error("Found no correct memorytype");
}

void createBuffer(VkDevice device, VkPhysicalDevice physDevice, VkDeviceSize deviceSize, VkBufferUsageFlags bufferUsageFlags, VkBuffer &buffer, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory &deviceMemory) {
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = deviceSize;
	bufferCreateInfo.usage = bufferUsageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	ASSERT_VK(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer));

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(physDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	ASSERT_VK(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory));

	ASSERT_VK(vkBindBufferMemory(device, buffer, deviceMemory, 0));
}

template<typename T>
void createAndUploadBuffer(VkDevice device, VkPhysicalDevice physDevice, VkQueue queue, VkCommandPool commandPool, 
	std::vector<T> data, VkBufferUsageFlags bufferUsageFlags, VkBuffer &buffer, VkDeviceMemory deviceMemory) {
	VkDeviceSize bufferSize = sizeof(T) *data.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(device, physDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBufferMemory);

	void *rawData;
	ASSERT_VK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &rawData));
	memcpy(rawData, data.data(), bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(device, physDevice, bufferSize, bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, deviceMemory);

	//copy
	copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}
