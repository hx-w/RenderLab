#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

out vec3 FragPos; 
out vec3 near;
out vec3 far;

void main() {
    FragPos.xz = (aPos.xz * 200.0f) + viewPos.xz;
    FragPos.y = -10.0f;
    gl_Position = projection * view * vec4(FragPos, 1.0f);
}
