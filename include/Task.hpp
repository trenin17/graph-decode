#include "Decoder.hpp"
#include <boost/fiber/fiber.hpp>

class Task {
public:
    Task(std::unique_ptr<Decoder>&& decoder) {
        fiber = std::make_unique<boost::fibers::fiber>([ d = std::move(decoder)]() mutable {
            d->Run();
        });
    }

    void Finish() {
        fiber->join();
    }

private:
    std::unique_ptr<boost::fibers::fiber> fiber;
};