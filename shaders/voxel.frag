#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in float fragConcentration;

out vec4 fragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    float ambient = 0.5;

    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(fragNormal, lightDir), 0.0);

    fragColor = vec4(vec3(0.0118, 0.9882, 0.4510) * (diff + ambient), fragConcentration);
}
