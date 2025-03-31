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
    Engine(size_t pool_size) : mem_pool_size(pool_size), input{pool_size}, output{pool_size} {}

    void init() {
        std::vector<PacketQueuePtr> fm_outgoing = {PacketQueuePtr(output)};
        PacketQueue& qincoming = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        
        //queues.back()->pop();
        PacketQueuePtr fm_incoming(qincoming);
        std::unique_ptr<Decoder> final_merger = std::make_unique<Decoder>("0", fm_incoming, fm_outgoing);
        decoders.push_back(std::move(final_merger));

        PacketQueue& q1 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q2 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q3 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        decoders.push_back(std::make_unique<Decoder>("1", PacketQueuePtr{q1}, std::vector{PacketQueuePtr(fm_incoming)}));
        decoders.push_back(std::make_unique<Decoder>("2", PacketQueuePtr{q2}, std::vector{PacketQueuePtr(fm_incoming)}));
        decoders.push_back(std::make_unique<Decoder>("3", PacketQueuePtr{q3}, std::vector{PacketQueuePtr(fm_incoming)}));

        PacketQueue& q4 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q5 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q6 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        decoders.push_back(std::make_unique<Decoder>("4", PacketQueuePtr{q4}, std::vector{PacketQueuePtr(q1)}));
        decoders.push_back(std::make_unique<Decoder>("5", PacketQueuePtr{q5}, std::vector{PacketQueuePtr(q2)}));
        decoders.push_back(std::make_unique<Decoder>("6", PacketQueuePtr{q6}, std::vector{PacketQueuePtr(q3)}));

        PacketQueue& q7 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q8 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q9 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        decoders.push_back(std::make_unique<Decoder>("7", PacketQueuePtr{q7}, std::vector{PacketQueuePtr(q4)}));
        decoders.push_back(std::make_unique<Decoder>("8", PacketQueuePtr{q8}, std::vector{PacketQueuePtr(q5)}));
        decoders.push_back(std::make_unique<Decoder>("9", PacketQueuePtr{q9}, std::vector{PacketQueuePtr(q6)}));

        PacketQueue& q10 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q11 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q12 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        decoders.push_back(std::make_unique<Decoder>("10", PacketQueuePtr{q10}, std::vector{PacketQueuePtr(q7)}));
        decoders.push_back(std::make_unique<Decoder>("11", PacketQueuePtr{q11}, std::vector{PacketQueuePtr(q8)}));
        decoders.push_back(std::make_unique<Decoder>("12", PacketQueuePtr{q12}, std::vector{PacketQueuePtr(q9)}));


        PacketQueue& q13 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q14 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q15 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        decoders.push_back(std::make_unique<Decoder>("13", PacketQueuePtr{q13}, std::vector{PacketQueuePtr(q10)}));
        decoders.push_back(std::make_unique<Decoder>("14", PacketQueuePtr{q14}, std::vector{PacketQueuePtr(q11)}));
        decoders.push_back(std::make_unique<Decoder>("15", PacketQueuePtr{q15}, std::vector{PacketQueuePtr(q12)}));

        PacketQueue& q16 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q17 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        PacketQueue& q18 = *queues.emplace_back(std::make_unique<PacketQueue>(mem_pool_size));
        decoders.push_back(std::make_unique<Decoder>("16", PacketQueuePtr{q16}, std::vector{PacketQueuePtr(q13)}));
        decoders.push_back(std::make_unique<Decoder>("17", PacketQueuePtr{q17}, std::vector{PacketQueuePtr(q14)}));
        decoders.push_back(std::make_unique<Decoder>("18", PacketQueuePtr{q18}, std::vector{PacketQueuePtr(q15)}));
        
        auto long_processing = [](BlobPacket packet) {
            std::random_device rd;
            std::mt19937 g(rd());

            std::vector<size_t> hashes;
            for (int i = 0; i < 3; i++) {
                hashes.push_back(std::hash<std::string>{}(packet.data));
                std::shuffle(packet.data, packet.data + 127, g);  
            }
        };
        decoders.push_back(std::make_unique<Decoder>("19", PacketQueuePtr{input}, std::vector{PacketQueuePtr(q16), PacketQueuePtr(q17), PacketQueuePtr(q18)}, long_processing));
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
    PacketQueue input;
    std::vector<std::unique_ptr<Decoder>> decoders;
    std::vector<std::unique_ptr<PacketQueue>> queues;
    std::vector<Task> tasks;
};