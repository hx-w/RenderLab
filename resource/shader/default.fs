#version 330 core

in vec3 FragPos;
in vec3 objectColor;
in vec3 Norm;
out vec4 FragColor;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool ignoreLight;

void main() {
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

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
