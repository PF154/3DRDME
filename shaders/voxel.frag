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

    float diff = 0.3 * max(dot(fragNormal, lightDir), 0.0);

    if (fragConcentration < 0.15) discard;

    fragColor = vec4(mix(vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), fragConcentration) * (diff + ambient), 1.0);
}
