#include "stdafx.h"

#include "Utils.h"
#include "Settings.h"


void notSupported(const char* msg) {
	
	throw new std::exception(msg);
}

uint32_t findMemoryTypeIndex(const uint32_t typeFilter, const VkMemoryPropertyFlags memoryPropertyFlags) {
	
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(Settings::getPhysDevice(), &physicalDeviceMemoryProperties);//Check validity
	
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
		
		if ((typeFilter & (1 << i)) && ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)) {//Research on memory types
			
			return i;
		}
	}

	throw std::runtime_error("Found no correct memorytype");
}

bool isFormatSupported(const VkFormat format, const VkImageTiling imageTiling, const VkFormatFeatureFlags formatFeatureFlags) {

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(Settings::getPhysDevice(), format, &formatProperties);

	if (imageTiling == VK_IMAGE_TILING_LINEAR && ((formatProperties.linearTilingFeatures & formatFeatureFlags) == formatFeatureFlags)) {
		
		return true;
	}
	else if (imageTiling == VK_IMAGE_TILING_OPTIMAL &&((formatProperties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags)) {

		return true;
	}
	else {
		return false;
	}
}

VkFormat findSupportedFormat(const std::vector<VkFormat> formats, const VkImageTiling imageTiling, const VkFormatFeatureFlags formatFeatureFlags) {

	for (VkFormat format : formats) {

		if (isFormatSupported( format, imageTiling, formatFeatureFlags)) {

			return format;
		}
	}

	throw new std::exception("No supported format found!");
}

bool isStencilFormat(const VkFormat format) {

	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void createBuffer( const VkDeviceSize deviceSize, const VkBufferUsageFlags bufferUsageFlags,  VkBuffer &buffer, 
				   const VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory &deviceMemory) {
	
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = deviceSize;
	bufferCreateInfo.usage = bufferUsageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	ASSERT_VK(vkCreateBuffer(Settings::getDevice(), &bufferCreateInfo, nullptr, &buffer));

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(Settings::getDevice(), buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex( memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	ASSERT_VK(vkAllocateMemory(Settings::getDevice(), &memoryAllocateInfo, nullptr, &deviceMemory));

	ASSERT_VK(vkBindBufferMemory(Settings::getDevice(), buffer, deviceMemory, 0));
}

void copyBuffer(const VkBuffer src, const VkBuffer dest, const VkDeviceSize deviceSize) {

	const VkCommandBuffer commandBuffer = startOneTimeCommandBuffer();

	VkBufferCopy bufferCopy;
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = deviceSize;

	vkCmdCopyBuffer(commandBuffer, src, dest, 1, &bufferCopy);

	endOneTimeCommandBuffer( Settings::getTransferQueue(), Settings::getCommandPool(), commandBuffer);

}

VkCommandBuffer startOneTimeCommandBuffer() {
	
	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = Settings::getCommandPool();
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	ASSERT_VK(vkAllocateCommandBuffers(Settings::getDevice(), &commandBufferAllocateInfo, &commandBuffer));

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	ASSERT_VK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

	return commandBuffer;
}

void endOneTimeCommandBuffer(const VkQueue queue, const VkCommandPool commandPool, const VkCommandBuffer commandBuffer) {
	
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

	ASSERT_VK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));// get Queue with only transfer bit

	vkQueueWaitIdle(queue);

	//optimization with fences

	vkFreeCommandBuffers(Settings::getDevice(), commandPool, 1, &commandBuffer);
}

void createImage(const uint32_t width, const uint32_t height, const VkFormat format, const VkImageTiling imageTiling,
	const VkImageUsageFlags imageUsageFlags, const VkMemoryPropertyFlags memoryPropertyFlags, VkImage &image, VkDeviceMemory &imageMemory) {

	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = imageTiling;
	imageCreateInfo.usage = imageUsageFlags;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	ASSERT_VK(vkCreateImage(Settings::getDevice(), &imageCreateInfo, nullptr, &image));

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(Settings::getDevice(), image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex( memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	ASSERT_VK(vkAllocateMemory(Settings::getDevice(), &memoryAllocateInfo, nullptr, &imageMemory));

	vkBindImageMemory(Settings::getDevice(), image, imageMemory, 0);
}

VkImageView createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags imageAspectFlags) {
	
	VkImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = imageAspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView view;
	ASSERT_VK(vkCreateImageView(Settings::getDevice(), &imageViewCreateInfo, nullptr, &view));

	return view;
}

void changeImageLayout(const VkQueue queue, const VkImage image, const VkFormat format, const VkImageLayout oldImageLayout, const VkImageLayout newImageLayout) {
	
	const VkCommandBuffer commandBuffer = startOneTimeCommandBuffer();

	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image;
	if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {

		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (isStencilFormat(format)) {

			imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {

		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	if (oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}	else if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else if (oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {

		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	} else {
		
		throw new std::exception("Layout transition failed!");
	}

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	
	endOneTimeCommandBuffer( queue, Settings::getCommandPool(), commandBuffer);
}

std::vector<char> readBytesFromFile(const std::string& filename) {

	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file) {

		std::cerr << "Could not open shader!" << std::endl;
		throw std::runtime_error("Could not open shaderfile!");
	}

	const size_t size = (size_t)file.tellg();
	std::vector<char> fileBuffer(size);
	file.seekg(0);
	file.read(fileBuffer.data(), size);
	file.close();

	return fileBuffer;
}

void createShaderModule(const std::vector<char> &code, VkShaderModule* const shaderModule) {

	VkShaderModuleCreateInfo shaderModuleCreateInfo;
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = (uint32_t*)code.data();

	ASSERT_VK(vkCreateShaderModule(Settings::getDevice(), &shaderModuleCreateInfo, nullptr, shaderModule));
}

void loadShader(const std::string& filename, VkShaderModule* const shaderModule) {

	std::vector<char> code = readBytesFromFile(filename);
	createShaderModule(code, shaderModule);
}