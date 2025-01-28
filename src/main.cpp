#include <boost/fiber/all.hpp>
#include <iostream>
#include "../include/Engine.hpp"

boost::fibers::condition_variable cv;
boost::fibers::mutex mt;
bool ready = false;

void thread_task(int num_threads, int current)
{
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
    std::unique_lock<boost::fibers::mutex> guard(mt);
    while (!ready) {
        cv.wait(guard);
    }
}

#include <chrono>

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    // boost::this_fiber::sleep_for(std::chrono::seconds(10));
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
    Engine engine;
    std::cout << "Initializing" << std::endl;
    engine.init();
    std::cout << "Running" << std::endl;
    engine.run();

    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads - 1; ++i) {
        threads.emplace_back([num_threads, i]() { thread_task(num_threads, i); });
    }

    std::cout << "Pushing input" << std::endl;
    for (int i = 0; i < 1000000; i++) {
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
