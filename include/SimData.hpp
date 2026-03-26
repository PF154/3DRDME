#include <array>
#include <vector>
#include <random>
#include <algorithm>
#include <future>
#include <chrono>

struct SimData 
{
    std::array<std::array<std::array<float, 100>, 100>, 100> ping;
    std::array<std::array<std::array<float, 100>, 100>, 100> pong;

    bool pingCurrent = true;

    std::future<void> computeFuture;

    SimData();

    float* getSimData();

    std::array<std::array<std::array<float, 100>, 100>, 100>* getBackBuffer();

    void compute(std::array<std::array<std::array<float, 100>, 100>, 100>& buffer);

    bool update();
};