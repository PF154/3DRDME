#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in float fragConcentration;

out vec4 fragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    float ambient = 0.25;

    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(fragNormal, lightDir), 0.0);

    fragColor = vec4(vec3(1.0, 1.0, 0.5) * (diff + ambient), fragConcentration);
}
