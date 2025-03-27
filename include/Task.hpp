#include "Decoder.hpp"
#include <thread>

class Task {
public:
    Task(std::unique_ptr<Decoder>&& decoder) {
        fiber = std::make_unique<std::thread>([ d = std::move(decoder)]() mutable {
            d->Run();
        });
    }

    void Finish() {
        fiber->join();
    }

private:
    std::unique_ptr<std::thread> fiber;
};