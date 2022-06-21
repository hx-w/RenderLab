#version 330 core

uniform vec3 P;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertexPosition; 

void main() {
    vertexPosition = P.xyz;
    gl_Position = projection * view * model * vec4(P, 1.0f);
}
