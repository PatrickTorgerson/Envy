#include <bench.hpp>

namespace Envy
{

    bench::bench(std::string name) :
        name {std::move(name)}
    { }


    void bench::start()
    {
        t = std::chrono::high_resolution_clock::now();
    }


    f64 bench::record()
    {
        std::chrono::duration<f64> delta { std::chrono::high_resolution_clock::now() - t };
        times.push_back(delta.count());
        return delta.count();
    }


    void bench::clear()
    { times.clear(); }


    std::string bench::to_string()
    {
        if(times.empty())
        { return name + "(0)"; }

        f64 min   {times.front()};
        f64 max   {times.front()};
        f64 mean  {};

        for(auto t : times)
        {
            mean += t;

            min = std::min(min, t);
            max = std::max(max, t);
        }

        mean /= (f64) times.size();

        return std::format("{: <10} x{:04} [{:.2f}s,{:.2f}s] | {:.4f}s", name, times.size(), min, max, mean);
    }

}