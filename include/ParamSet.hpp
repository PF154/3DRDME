#pragma once

#include <fstream>

struct ParamSet {
    float f;   // Feed rate
    float k;   // Kill rate
    float du;  // Diffusion rate for u
    float dv;  // Diffusion rate for v
    float dx;  // Spatial step
    float dt;  // Time step

    // Reads params.txt if it exists, otherwise uses provided defaults.
    // params.txt format: one "key value" pair per line, e.g. "f 0.035"
    ParamSet(float f, float k, float du, float dv, float dx, float dt)
        : f(f), k(k), du(du), dv(dv), dx(dx), dt(dt)
    {
        std::ifstream file("params.txt");
        if (!file.is_open()) return;

        std::string key;
        float value;
        while (file >> key >> value)
        {
            if      (key == "f")  this->f  = value;
            else if (key == "k")  this->k  = value;
            else if (key == "du") this->du = value;
            else if (key == "dv") this->dv = value;
            else if (key == "dx") this->dx = value;
            else if (key == "dt") this->dt = value;
        }
    }
};