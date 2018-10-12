#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 viewingDir;
layout(location = 4) in vec3 lightingDir;

layout (location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D tex;

void main(){
	outColor= vec4(color, 1.0);
	//outColor = texture(tex, UV);

	vec3 n = normalize(normal);
	vec3 l = normalize(lightingDir);
	vec3 v = normalize(viewingDir);
	vec3 r = reflect(-l, n);

	vec3 ambient = color * 0.1;
	vec3 diffuse = max(dot(n, l), 0.0f) * color;
	vec3 specular = pow(max(dot(r, v), 0.0), 16.0) * vec3(1.0f);

	outColor = vec4(ambient + diffuse + specular, 1);


}