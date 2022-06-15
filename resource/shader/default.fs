#version 330 core
#ifdef GL_ES
precision mediump float;
#endif


in vec3 FragPos;
in vec3 objectColor;
in vec3 Norm;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool ignoreLight;
uniform bool randomColor;

uniform float u_time;   // Time in seconds since load

void main() {
    vec3 color = objectColor;
    if (randomColor) {
        color = vec3(
            sin(u_time + FragPos.x) * 0.5 + 0.5,
            sin(u_time + 2.0 + FragPos.y) * 0.5 + 0.5,
            sin(u_time + 4.0 + FragPos.z) * 0.5 + 0.5
        );
    }
    if (ignoreLight) {
        FragColor = vec4(objectColor, 1.0);
        return;
    }
    float ambientStrength = 1.0f;
    float specularStrength = 0.5f;

    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Norm);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.05);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.05), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0);
}
