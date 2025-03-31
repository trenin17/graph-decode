#include "Task.hpp"
#include <optional>
#include <memory>
#include <algorithm>

class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

class Engine {
public:
    Engine(size_t pool_size) : mem_pool_size(pool_size), output{pool_size} {}

    void init() {
        CreateNetwork1();
    }

    void CreateDecoder() {
        PacketQueue& q = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        decoders.push_back(std::make_unique<Decoder>(decoders.size(), PacketQueuePtr{q}, std::vector<PacketQueuePtr>{}));
    }

    void MakeEdge(size_t from, size_t to) {
        decoders[from]->AddOutgoing(decoders[to]->GetIncoming());
    }

    void CreateNetwork1() {
        for (size_t i = 0; i < 20; i++) {
            CreateDecoder();
        }
        for (size_t i = 0; i < 3; i++) {
            for (size_t j = 0; j < 6; j++) {
                size_t to = i * 6 + j;
                if (j == 0) to = 0;
                MakeEdge(i * 6 + j + 1, to);
            }
        }
        for (size_t i = 0; i < 3; i++) {
            MakeEdge(19, (i+1) * 6);
        }
        decoders[0]->AddOutgoing(output);
        
        
        input = decoders[19]->GetIncoming();
        decoders[19]->AddPacketHandler(long_processing);
        input.add_producers(1);
        for (auto it = decoders.rbegin(); it != decoders.rend(); ++it) {
            (*it)->init();
        }
    }

    void run() {
        for (auto& decoder : decoders) {
            tasks.emplace_back(std::move(decoder));
        }
    }

    void pushInput(BlobPacket packet) {
        input.push(packet);
    }

    void waitForFinish() {
        for (auto& task : tasks) {
            task.Finish();
        }
    }

    BlobPacket popOutput(bool log = false) {
        if (log) {
            std::cout << "Pop output" << std::endl;
        }
        return output.pop();
    }

private:
    size_t mem_pool_size = 100;
    PacketQueue output;
    PacketQueuePtr input;
    std::vector<std::unique_ptr<Decoder>> decoders;
    std::vector<std::unique_ptr<PacketQueue>> queues;
    std::vector<Task> tasks;

    std::function<void(BlobPacket)> long_processing = [](BlobPacket packet) {
        std::random_device rd;
        std::mt19937 g(rd());

        std::vector<size_t> hashes;
        for (int i = 0; i < 3; i++) {
            hashes.push_back(std::hash<std::string>{}(packet.data));
            std::shuffle(packet.data, packet.data + 127, g);  
        }
    };
};