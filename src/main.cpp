#include <boost/thread.hpp>
#include <iostream>
#include "../include/Engine.hpp"

#include <chrono>

int main(int argc, char *argv[]) {
    // std::this_thread::sleep_for(std::chrono::seconds(10));

    static_assert(sizeof(BlobPacket) == 128, "BlobPacket size is not 128 bytes");

    auto start = std::chrono::high_resolution_clock::now();

    size_t num_iterations = 0;
    size_t memory_pool_size = 0;
    size_t network_id = 1;
    if (argc >= 3) {
        num_iterations = std::stoull(argv[1]);
        memory_pool_size = std::stoull(argv[2]);
        if (argc >= 4) {
            network_id = std::stoull(argv[3]);
        }
    } else {
        std::cerr << "Usage: " << argv[0] << " <num_iterations> <memory_pool_size> <network_id>" << std::endl;
        return 1;
    }

    //boost::this_fiber::sleep_for(std::chrono::seconds(10));
    Engine engine{memory_pool_size};
    std::cout << "Initializing" << std::endl;
    engine.init(network_id);
    std::cout << "Running" << std::endl;
    engine.run();

    std::cout << "Pushing input" << std::endl;

    size_t n_total_packets = 0;

    auto final_fiber = std::make_unique<boost::thread>([&]() mutable {
        auto eof_cnt = 0;
        while (true) {
            auto packet = engine.popOutput();
            // n_total_packets++;
            //std::cout << packet.data << std::endl;
            if (packet.is_eof) {
                eof_cnt++;
                // std::cout << "EOF " << eof_cnt << std::endl;
                if (eof_cnt == engine.GetProducersCount()) {
                    break;
                }
            }
        }
    });

    for (int i = 0; i < num_iterations; i++) {
        BlobPacket packet;
        for (int j = 0; j < PACKET_DATA_SIZE; j++) {
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
    // std::cout << "Total packets processed: " << n_total_packets << std::endl;
    std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;
    
    return 0;
}
