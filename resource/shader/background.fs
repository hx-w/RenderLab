#version 330 core

in vec3 FragPos;

uniform vec3 viewPos;

out vec4 FragColor;

void main() {
	float scale = 0.5f;

	int c = (
		int(round(FragPos.x * scale)) +
		int(round(FragPos.z * scale))
	) % 2;

	if (c == 0) {
		FragColor = vec4(0.3f, 0.3f, 0.3f, 0.8f);
	} else {
		FragColor = vec4(0.8f, 0.8f, 0.8f, 0.8f);
	}
}