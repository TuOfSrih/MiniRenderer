
#pragma once


class Pipeline
{
public:

	Pipeline();
	~Pipeline();

	void init(VkShaderModule vertexShader, VkShaderModule fragmentShader, uint32_t width, uint32_t height);
	void create(VkDevice device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	void destroy();

private:
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;

	bool initialize

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfoVert;
	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfoFrag;

	VkVertexInputBindingDescription vertexInputBindingDescription = Vertex::getVertexInputBindingDescription();
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescritions = Vertex::getVertexInputAttributDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo;
	VkViewport viewPort;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo;
	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo;
	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo;
	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfoOpague = DepthImage::getPipelineDepthStencilStateCreateInfoOpaque();
	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState;
	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo;
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo;
	VkPushConstantRange pushConstantRange;

};

