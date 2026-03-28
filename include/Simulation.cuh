#pragma once

#include <ParamSet.hpp>

#include <cuda_runtime.h>
#include <vector>

class Simulation {
private:
    // float buffering
    float *d_u_ping, *d_v_ping;
    float *d_u_pong, *d_v_pong;
    int active_buffer = 0;

    // Simulation parameters
    int N;                      // Size of cubic grid
    ParamSet params;            // f, k, du, dv, dx, dt
    int current_timestep;

    // CUDA grid configuration
    dim3 threads;              // Threads per block (e.g., 16x16)
    dim3 blocks;               // Number of blocks

    // Helper methods
    void allocate_gpu_memory();
    void free_gpu_memory();
    void initialize_perturbation();
    float calculate_stable_dt(float dx, float du, float dv);

public:
    /**
     * Constructor: Initialize simulation with given parameters and grid size.
     *
     * @param p Gray-Scott parameters (f, k, du, dv)
     * @param grid_size Grid dimension (creates grid_size x grid_size grid)
     */
    Simulation(ParamSet p, int grid_size = 100);

    ~Simulation();

    /**
     * Run N simulation timesteps using ping-pong buffers.
     *
     * @param num_steps Number of timesteps to execute
     */
    void step(int num_steps);

    /**
     * Copy current simulation state from GPU to CPU.
     *
     * @param u_out Output vector for U field (will be resized to Nx*Ny)
     * @param v_out Output vector for V field (will be resized to Nx*Ny)
     */
    void get_state(std::vector<float>& u_out, std::vector<float>& v_out);

    /**
     * Reset simulation to initial condition.
     */
    void reset();

    /**
     * Get current timestep number.
     */
    int get_timestep() const { return current_timestep; }

    /**
     * Get simulation parameters.
     */
    ParamSet get_params() const { return params; }
};