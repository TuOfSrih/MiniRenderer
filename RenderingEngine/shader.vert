#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex{
	vec4 gl_Position;
};

layout(binding = 0) uniform MATRICES {
	//mat4 model;
	//mat4 modelView;
	mat4 modelViewProjection;
} matrices;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;



void main(){
	vec4 x = matrices.modelViewProjection * vec4(pos, 0.0, 1.0);
	//gl_Position = vec4(pos, 0.0f, 1.0f);
	gl_Position = matrices.modelViewProjection * vec4(pos, 0.0, 1.0);
	fragColor = color;

}