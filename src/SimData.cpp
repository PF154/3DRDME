#include <SimData.hpp>

#include <array>
#include <vector>
#include <random>
#include <algorithm>
#include <future>
#include <chrono>

SimData::SimData()
{
    // Make sure that ping is populated at startup, since we're going to be using it
    std::mt19937 engine(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int i = 0; i < 100; ++i)
        for (int j = 0; j < 100; ++j)
            for (int k = 0; k < 100; ++k)
                ping[i][j][k] = dist(engine);
}


float* SimData::getSimData()
{
    return pingCurrent ? &ping[0][0][0] : &pong[0][0][0];
}

std::array<std::array<std::array<float, 100>, 100>, 100>* SimData::getBackBuffer()
{
    return pingCurrent ? &pong : &ping;
}

// Generate new data in place in buffer
void SimData::compute(std::array<std::array<std::array<float, 100>, 100>, 100>& buffer)
{
    std::mt19937 engine(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int i = 0; i < 100; ++i)
        for (int j = 0; j < 100; ++j)
            for (int k = 0; k < 100; ++k)
                buffer[i][j][k] = dist(engine);

}

// Generate data on back buffer when necessary
bool SimData::update()
{
    if (!computeFuture.valid())
    {
        computeFuture = std::async(
            std::launch::async, 
            &SimData::compute, 
            this, 
            std::ref(*getBackBuffer())
        );
        return false;
    }

    if (computeFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        computeFuture = std::async(
            std::launch::async, 
            &SimData::compute, 
            this, 
            std::ref(*getBackBuffer())
        );
        pingCurrent = !pingCurrent;

        return true;
    }

    return false;
}