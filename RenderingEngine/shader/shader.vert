#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex{
	vec4 gl_Position;
};

layout(binding = 0) uniform MATRICES {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 lightPos;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 UV;
layout(location = 3) in vec3 normals;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragViewingDir;
layout(location = 4) out vec3 fragLightDir;



void main(){

	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);
	vec4 worldPos = ubo.model * vec4(pos, 1.0);
	fragColor = color;
	fragUV = UV;	
	fragNormal = mat3(ubo.view) * mat3(ubo.model) * normals;
	fragViewingDir = -(ubo.view * worldPos).xyz;
	fragLightDir = mat3(ubo.view) * (ubo.lightPos - vec3(worldPos));
}