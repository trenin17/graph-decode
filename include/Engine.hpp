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

    void init(size_t network = 1) {
        switch (network) {
            case 1:
                CreateNetwork1();
                break;
            case 2:
                CreateNetwork2();
                break;
            case 3:
                CreateNetwork3();
                break;
            case 4:
                CreateNetwork4();
                break;
            case 5:
                CreateNetwork5();
                break;
            default:
                CreateNetwork1();
        }

        decoders[0]->AddOutgoing(output);
        input.add_producers(1);
        for (auto it = decoders.rbegin(); it != decoders.rend(); ++it) {
            (*it)->init();
        }
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
        
        input = decoders[19]->GetIncoming();
        decoders[19]->AddPacketHandler(long_processing);
    }

    void CreateNetwork2() {
        for (size_t i = 0; i < 302; i++) {
            CreateDecoder();
        }
        for (size_t i = 0; i < 3; i++) {
            for (size_t j = 0; j < 100; j++) {
                size_t to = i * 100 + j;
                if (j == 0) to = 0;
                MakeEdge(i * 100 + j + 1, to);
            }
        }
        for (size_t i = 0; i < 3; i++) {
            MakeEdge(301, (i+1) * 100);
        }
        
        input = decoders[301]->GetIncoming();
        decoders[301]->AddPacketHandler(long_processing);
    }

    void CreateNetwork3() {
        CreateNetwork2();
        decoders[300]->AddPacketHandler(long_processing);
        decoders[200]->AddPacketHandler(long_processing);
        decoders[100]->AddPacketHandler(long_processing);
    }

    void CreateNetwork4() {
        for (int i = 0; i < 11; i++) {
            CreateDecoder();
        }
        size_t input_decoder = 10;
        size_t output_decoder = 0;
        std::vector<size_t> input_layer = {7, 8, 9};
        std::vector<size_t> output_layer = {1, 2};
        std::vector<size_t> hidden_layer = {3, 4, 5, 6};

        for (auto i : input_layer) {
            MakeEdge(input_decoder, i);
        }

        for (auto i : input_layer) {
            for (auto j : hidden_layer) {
                MakeEdge(i, j);
            }
        }

        for (auto i : hidden_layer) {
            for (auto j : output_layer) {
                MakeEdge(i, j);
            }
        }

        for (auto i : output_layer) {
            MakeEdge(i, output_decoder);
        }

        input = decoders[input_decoder]->GetIncoming();
    }

    void CreateNetwork5() {
        // Bamboo
        for (int i = 0; i < 100; i++) {
            CreateDecoder();
        }

        for (int i = 0; i < 99; i++) {
            MakeEdge(i + 1, i);
        }

        input = decoders[99]->GetIncoming();
        decoders[99]->AddPacketHandler(long_processing);
    }

    void run() {
        for (auto& decoder : decoders) {
            tasks.emplace_back(std::move(decoder));
        }
    }

    void pushInput(const BlobPacket& packet) {
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

    size_t GetProducersCount() {
        return output.get_producers();
    }

private:
    size_t mem_pool_size = 100;
    PacketQueue output;
    PacketQueuePtr input;
    std::vector<std::unique_ptr<Decoder>> decoders;
    std::vector<std::unique_ptr<PacketQueue>> queues;
    std::vector<Task> tasks;

    std::function<void(const BlobPacket&)> long_processing = [](const BlobPacket& packet) {
        BlobPacket copy = packet;
        std::random_device rd;
        std::mt19937 g(rd());

        std::vector<size_t> hashes;
        for (int i = 0; i < 3; i++) {
            hashes.push_back(std::hash<std::string>{}(copy.data));
            std::shuffle(copy.data, copy.data + 127, g);  
        }
    };
};