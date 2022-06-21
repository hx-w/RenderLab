#version 330 core

in vec3 FragPos;

out vec4 FragColor;

void main() {
	float scale = 5.0f;

	int c = (
		int(round(FragPos.x * 10.0)) +
		int(round(FragPos.z * 10.0))
	) % 2;

	if (c == 0) {
		FragColor = vec4(0.3f, 0.3f, 0.3f, 1.0f);
	} else {
		FragColor = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	}
}