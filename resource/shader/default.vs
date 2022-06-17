#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNorm;

out vec3 objectColor;
out vec3 Norm;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 offset;

void main() {
    objectColor = aColor;
    Norm = aNorm;
    vec3 pos = aPos + offset;
    FragPos = vec3(model * vec4(pos, 1.0));
    gl_Position = projection * view * model * vec4(pos, 1.0f);
}
