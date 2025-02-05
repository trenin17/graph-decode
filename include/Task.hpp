#include "Decoder.hpp"

class Task {
public:
    Task(std::unique_ptr<Decoder>&& decoder) {
        // fiber = std::make_unique<boost::fibers::fiber>([ d = std::move(decoder)]() mutable {
            // decoder->Run();
        // });
    }

    void Finish() {
        // fiber->join();
    }

private:
};