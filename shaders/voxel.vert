#version 330 core

// Per-vertex attributes (from cube geometry)
layout(location = 0) in vec3 aPos;

// Per-instance attribute (one per voxel, advances once per instance)
layout(location = 1) in vec3 aInstancePos;

// Face normals
layout (location = 2) in vec3 aNormal;

// Voxel concentration
layout (location = 3) in float aConcentration;

// Transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Ouputs
out vec3 fragPos;
out vec3 fragNormal;
out float fragConcentration;

void main()
{
    fragPos = vec3(model * vec4(aPos + aInstancePos, 1.0));
    fragNormal = aNormal;
    fragConcentration = aConcentration;
    gl_Position = projection * view * model * vec4(aPos + aInstancePos, 1.0);
}
