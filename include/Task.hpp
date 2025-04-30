#include "Decoder.hpp"
#include <boost/thread.hpp>
#include <sstream>


class Task {
public:
    Task(std::unique_ptr<Decoder>&& decoder) {
        fiber = std::make_unique<boost::thread>([ d = std::move(decoder)]() mutable {
            // std::stringstream ss;
            // ss << d->id << " " << std::this_thread::get_id() << '\n';
            // std::string ids = ss.str();
            // std::cout << ids;
            
            d->Run();
        });
    }

    void Finish() {
        fiber->join();
    }

private:
    std::unique_ptr<boost::thread> fiber;
};