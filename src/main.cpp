#include <boost/fiber/all.hpp>
#include <iostream>
#include "../include/Engine.hpp"

boost::fibers::condition_variable cv;
boost::fibers::mutex mt;
bool ready = false;

void bind_to_core(DWORD core_id)
{
    HANDLE thread           = GetCurrentThread();  // Get handle to the current thread
    DWORD_PTR affinity_mask = 1ll << core_id;      // Create a bitmask with the desired core
    SetThreadAffinityMask(thread, affinity_mask);  // Bind thread to core
}

void thread_task(int num_threads, int current)
{
    bind_to_core(current);
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
    std::unique_lock<boost::fibers::mutex> guard(mt);
    while (!ready) {
        cv.wait(guard);
    }
}

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
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
    Engine engine{memory_pool_size};
    std::cout << "Initializing" << std::endl;
    engine.init();
    std::cout << "Running" << std::endl;
    engine.run();

    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads - 1; ++i) {
        threads.emplace_back([num_threads, i]() { thread_task(num_threads, i); });
    }
    bind_to_core(num_threads - 1);

    std::cout << "Pushing input" << std::endl;

    auto final_fiber = std::make_unique<boost::fibers::fiber>([&]() mutable {
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

    {
        std::unique_lock<boost::fibers::mutex> guard;
        ready = true;
        cv.notify_all();
    }
    for (auto &thread: threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;
    
    return 0;
}
