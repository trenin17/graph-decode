#include "Decoder.hpp"
#include <thread>
#include <sstream>


class Task {
public:
    Task(std::unique_ptr<Decoder>&& decoder) {
        fiber = std::make_unique<std::thread>([ d = std::move(decoder)]() mutable {
            std::stringstream ss;
            ss << d->id << " " << std::this_thread::get_id() << '\n';
            std::string ids = ss.str();
            std::cout << ids;
            
            d->Run();
        });
    }

    void Finish() {
        fiber->join();
    }

private:
    std::unique_ptr<std::thread> fiber;
};