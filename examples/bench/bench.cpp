#include <Envy/application.hpp>
#include <Envy/log.hpp>
#include <Envy/bench.hpp>

int main(int argc, char** argv)
{
    Envy::application app;

    Envy::info("The program is about to begin!");

    Envy::bench benchmark {"benchmarks"};
    benchmark.start();

    Envy::logger log("Bench", "logs.txt");

    log.error("A terrible error has occured!");
    log.note("It was a verry bad error");
    log.note("Devestation ensued");
    log.info("Okay I'm done now");
    Envy::error("Bench has reported an error!");
    Envy::note("I guess it was a pretty bad one too");

    Envy::info("Alright, let's wrap things up");
    benchmark.record();
    Envy::info(benchmark);


    return 0;
}