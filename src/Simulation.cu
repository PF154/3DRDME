#include <definitions.cuh>
#include <Simulation.cuh>

#include <cuda_runtime.h>
#include <vector>
#include <random>
#include <algorithm>

static __device__ float laplace_3d(
    const float* data,
    int x, int y, int z,
    int N
) {
    int left = ((x - 1 + N) % N) + y*N + z*N*N;
    int right = ((x + 1) % N) + y*N + z*N*N;
    int up = x + ((y - 1 + N) % N) * N + z*N*N;
    int down = x + ((y + 1) % N) * N + z*N*N;
    int front = x+ y*N + ((z - 1 + N) % N)*N*N;
    int rear = x+ y*N + ((z + 1) % N)*N*N;
    
    int idx = x + N * y + N*N * z;
    return data[left] + data[right] + data[up] + data[down] + data[rear] + data[front] -
           6.0 * data[idx];
}

__global__ void compute_time_step(
    const float* u_in,
    const float* v_in,
    float* u_out,
    float* v_out,
    int N,
    float f, float k,
    float du, float dv,
    float dx, float dt
) {
    int x = threadIdx.x + blockIdx.x * blockDim.x;
    int y = threadIdx.y + blockIdx.y * blockDim.y;
    int z = threadIdx.z + blockIdx.z * blockDim.z;
    
    if (x >= N || y >= N || z >= N) return;
    
    int idx = z*N*N + y * N + x;
    float u = u_in[idx];
    float v = v_in[idx];
    
    float laplace_u = laplace_3d(u_in, x, y, z, N);
    float laplace_v = laplace_3d(v_in, x, y, z, N);
    
    u_out[idx] = u + dt * (du * laplace_u - u*v*v + f * (1-u));
    v_out[idx] = v + dt * (dv * laplace_v + u*v*v - v * (f + k));
}

Simulation::Simulation(ParamSet p, int grid_size)
    : params(p), N(grid_size), current_timestep(0)
{
    threads = dim3(8, 8, 8);
    blocks = dim3((N+7)/8, (N+7)/8, (N+7)/8); 

    allocate_gpu_memory();

    initialize_perturbation();
}

Simulation::~Simulation()
{
    free_gpu_memory();
}

void Simulation::allocate_gpu_memory()
{
    const int total_size = N*N*N * sizeof(float);

    gpuErrchk(cudaMalloc(&d_u_ping, total_size));
    gpuErrchk(cudaMalloc(&d_v_ping, total_size));
    gpuErrchk(cudaMalloc(&d_u_pong, total_size));
    gpuErrchk(cudaMalloc(&d_v_pong, total_size));
}

void Simulation::free_gpu_memory()
{
    gpuErrchk(cudaFree(d_u_ping));
    gpuErrchk(cudaFree(d_v_ping));
    gpuErrchk(cudaFree(d_u_pong));
    gpuErrchk(cudaFree(d_v_pong));
}

void Simulation::initialize_perturbation()
{
    // Create CPU buffers for initial U and V fields
    std::vector<float> u_init(N * N * N, 1.0);
    std::vector<float> v_init(N * N * N, 0.0);

    // Set a small 5x5 cube in the middle to v = 1.0
    for (int y = N/2 - 2; y <= N/2 + 2; ++y)
        for (int x = N/2 - 2; x <= N/2 + 2; ++x)
            for (int z = N/2 - 2; z <= N/2 + 2; ++z)
                v_init[z*N*N + y * N + x] = 1.0;

    // Copy initial conditions to GPU ping buffer (active_buffer starts at 0)
    gpuErrchk(cudaMemcpy(d_u_ping, u_init.data(), N*N*N * sizeof(float), cudaMemcpyHostToDevice));
    gpuErrchk(cudaMemcpy(d_v_ping, v_init.data(), N*N*N * sizeof(float), cudaMemcpyHostToDevice));

    // Initialize state
    active_buffer = 0;
    current_timestep = 0;
}

void Simulation::step(int num_steps)
{
    for (int i = 0; i < num_steps; i++)
    {
        if (active_buffer)
        {
            compute_time_step<<<blocks, threads>>>(
                d_u_pong, d_v_pong, d_u_ping, d_v_ping, N,
                params.f, params.k, params.du, params.dv, params.dx, params.dt 
            );
            active_buffer = 0;
        }
        else
        {
            compute_time_step<<<blocks, threads>>>(
                d_u_ping, d_v_ping, d_u_pong, d_v_pong, N,
                params.f, params.k, params.du, params.dv, params.dx, params.dt 
            );
            active_buffer = 1;
        }
        current_timestep++;
    }
    gpuErrchk(cudaDeviceSynchronize());
}

void Simulation::get_state(std::vector<float>& u_out, std::vector<float>& v_out)
{
    u_out.resize(N*N*N);
    v_out.resize(N*N*N);

    if (active_buffer)
    {
        gpuErrchk(cudaMemcpy(u_out.data(), d_u_pong, sizeof(float) * N*N*N, cudaMemcpyDeviceToHost));
        gpuErrchk(cudaMemcpy(v_out.data(), d_v_pong, sizeof(float) * N*N*N, cudaMemcpyDeviceToHost));
    }
    else
    { 
        gpuErrchk(cudaMemcpy(u_out.data(), d_u_ping, sizeof(float) * N*N*N, cudaMemcpyDeviceToHost));
        gpuErrchk(cudaMemcpy(v_out.data(), d_v_ping, sizeof(float) * N*N*N, cudaMemcpyDeviceToHost));
    }
}

void Simulation::reset()
{
    initialize_perturbation();
}