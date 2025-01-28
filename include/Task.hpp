#include "Decoder.hpp"

class Task {
public:
    Task(std::unique_ptr<Decoder>&& decoder) {
        thread = std::make_unique<std::thread>([ d = std::move(decoder)]() mutable {
            d->Run();
        });
    }

    void Finish() {
        thread->join();
    }

private:
    std::unique_ptr<std::thread> thread;
};