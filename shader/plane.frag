#version 460 core
out vec4 FragColor;
in vec2 WorldPos;

uniform sampler2D planeMap;

void main(){
	vec3 envColor = texture(planeMap, WorldPos).rgb;

	envColor = envColor / (envColor + vec3(1.0));
	envColor = pow(envColor, vec3(1.0/2.2));

	FragColor = vec4(envColor, 1.0);
}
