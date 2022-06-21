#version 330 core

out vec4 color;

in vec3 vertexPosition;

void main() {
	float c = (
		int(round(vertexPosition.x * 5.0)) +
		int(round(vertexPosition.y * 5.0))
	) % 2;

	color = vec4(vec3(c / 2.0 + 0.3), 1);
}