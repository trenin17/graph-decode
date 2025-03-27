#include <thread>
#include <iostream>
#include "../include/Engine.hpp"

#include <chrono>

int main(int argc, char *argv[]) {
    auto start = std::chrono::high_resolution_clock::now();

    size_t num_iterations = 0;
    size_t memory_pool_size = 0;
    if (argc >= 3) {
        num_iterations = std::stoull(argv[1]);
        memory_pool_size = std::stoull(argv[2]);
    } else {
        std::cerr << "Usage: " << argv[0] << " <num_iterations> <memory_pool_size>" << std::endl;
        return 1;
    }

    //boost::this_fiber::sleep_for(std::chrono::seconds(10));
    Engine engine{memory_pool_size};
    std::cout << "Initializing" << std::endl;
    engine.init();
    std::cout << "Running" << std::endl;
    engine.run();

    std::cout << "Pushing input" << std::endl;

    auto final_fiber = std::make_unique<std::thread>([&]() mutable {
        auto eof_cnt = 0;
        while (true) {
            auto packet = engine.popOutput();
            //std::cout << packet.data << std::endl;
            if (packet.is_eof) {
                eof_cnt++;
                // std::cout << "EOF " << eof_cnt << std::endl;
                if (eof_cnt == 3) {
                    break;
                }
            }
        }
    });

    for (int i = 0; i < num_iterations; i++) {
        BlobPacket packet;
        for (int j = 0; j < 127; j++) {
            packet.data[j] = 'a' + (i + j) % 26;
        }
        engine.pushInput(std::move(packet));
    }
    engine.pushInput({true, ""});
    std::cout << "Waiting for finish" << std::endl;
    
    final_fiber->join();
    engine.waitForFinish();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;
    
    return 0;
}
