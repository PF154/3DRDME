#pragma once

struct ParamSet {
    float f;   // Feed rate
    float k;   // Kill rate
    float du;  // Diffusion rate for u
    float dv;  // Diffusion rate for v
    float dx;  // Spatial step
    float dt;  // Time step
};