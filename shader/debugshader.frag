#version 460 core
out vec4 FragColor;

uniform sampler2D depthMap;
uniform float near_plane;
uniform float far_plane;

in vec2 texcoords;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * 0.1 * 100) / (100 + 0.1 - z * (100 - 0.1));	
}

void main(){
	float depthValue = texture(depthMap, texcoords).r;
	//FragColor = vec4(vec3(LinearizeDepth(depthValue) / 100), 1.0); // perspective
	FragColor = vec4(vec3(depthValue), 1.0);
	//FragColor = vec4(texcoords,0.0, 1.0);
}