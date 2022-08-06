#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 WorldPos;

void main(){
	WorldPos = aPos;
	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * model * vec4(WorldPos, 1.0f);
	gl_Position = clipPos;
}