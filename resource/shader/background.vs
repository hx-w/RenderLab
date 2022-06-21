#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos; 
out vec3 near;
out vec3 far;

void main() {
    FragPos = aPos.xyz * 50.0f;
    gl_Position = projection * view * vec4(FragPos, 1.0f);
}
