#version 330 core

// Per-vertex attributes (from cube geometry)
layout(location = 0) in vec3 aPos;

// Per-instance attribute (one per voxel, advances once per instance)
layout(location = 1) in vec3 aInstancePos;

// Transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos + aInstancePos, 1.0);
}
