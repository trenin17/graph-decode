#include <iostream>
#include "../include/Engine.hpp"

#include <chrono>

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    Engine engine;
    std::cout << "Initializing" << std::endl;
    engine.init();
    std::cout << "Running" << std::endl;
    engine.run();

    std::cout << "Pushing input" << std::endl;
    for (int i = 0; i < 10000000; i++) {
        engine.pushInput({i});
    }
    engine.pushInput({-1, true});
    std::cout << "Waiting for finish" << std::endl;
    std::cout << "Output:" << std::endl;
    while (true) {
        auto packet = engine.popOutput();
        //std::cout << packet.data << std::endl;
        if (packet.eof) {
            break;
        }
    }
    engine.waitForFinish();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;
    
    return 0;
}
