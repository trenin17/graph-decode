#include <iostream>
#include <chrono>
#include <thread>
#include "../include/Engine.hpp"
// #include <tbb/flow_graph.h>

int main() {
    // std::cerr << "Hello, World!" << std::endl;
    // std::this_thread::sleep_for(std::chrono::seconds(10));


    // tbb::flow::graph g;
    auto start = std::chrono::high_resolution_clock::now();

    Engine engine;
    std::cout << "Initializing" << std::endl;
    engine.initAndRun();


    std::cout << "Pushing input" << std::endl;
    for (int i = 0; i < 10000000; i++) {
        engine.pushInput({i});
    }
    engine.pushInput({-1, true});
    std::cout << "Waiting for finish" << std::endl;
    engine.waitForFinish();
    std::cout << "Output:" << std::endl;
    while (true) {
        auto packet = engine.popOutput();
        //std::cout << packet.data << std::endl;
        if (packet.eof) {
            break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;
    return 0;
}
