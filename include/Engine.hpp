#include "Task.hpp"
#include <optional>
#include <memory>

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
    void init() {
        std::vector<PacketQueuePtr> fm_outgoing = {PacketQueuePtr(output)};
        PacketQueue& qincoming = *queues.emplace_back(std::make_unique<PacketQueue>());
        
        //queues.back()->pop();
        PacketQueuePtr fm_incoming(qincoming);
        std::unique_ptr<Decoder> final_merger = std::make_unique<Decoder>(fm_incoming, fm_outgoing);
        decoders.push_back(std::move(final_merger));

        PacketQueue& q1 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q2 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q3 = *queues.emplace_back(std::make_unique<PacketQueue>());
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q1}, std::vector{PacketQueuePtr(fm_incoming)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q2}, std::vector{PacketQueuePtr(fm_incoming)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q3}, std::vector{PacketQueuePtr(fm_incoming)}));

        PacketQueue& q4 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q5 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q6 = *queues.emplace_back(std::make_unique<PacketQueue>());
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q4}, std::vector{PacketQueuePtr(q1)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q5}, std::vector{PacketQueuePtr(q2)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q6}, std::vector{PacketQueuePtr(q3)}));

        PacketQueue& q7 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q8 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q9 = *queues.emplace_back(std::make_unique<PacketQueue>());
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q7}, std::vector{PacketQueuePtr(q4)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q8}, std::vector{PacketQueuePtr(q5)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q9}, std::vector{PacketQueuePtr(q6)}));

        PacketQueue& q10 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q11 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q12 = *queues.emplace_back(std::make_unique<PacketQueue>());
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q10}, std::vector{PacketQueuePtr(q7)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q11}, std::vector{PacketQueuePtr(q8)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q12}, std::vector{PacketQueuePtr(q9)}));


        PacketQueue& q13 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q14 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q15 = *queues.emplace_back(std::make_unique<PacketQueue>());
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q13}, std::vector{PacketQueuePtr(q10)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q14}, std::vector{PacketQueuePtr(q11)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q15}, std::vector{PacketQueuePtr(q12)}));

        PacketQueue& q16 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q17 = *queues.emplace_back(std::make_unique<PacketQueue>());
        PacketQueue& q18 = *queues.emplace_back(std::make_unique<PacketQueue>());
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q16}, std::vector{PacketQueuePtr(q13)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q17}, std::vector{PacketQueuePtr(q14)}));
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{q18}, std::vector{PacketQueuePtr(q15)}));
        
        auto long_processing = [](Packet packet) {
            double packet_data = packet.data;
            for (int i = 0; i < 1000; i++) {
                packet_data = 10 + std::sqrt(packet_data + 10);
            }
        };
        decoders.push_back(std::make_unique<Decoder>(PacketQueuePtr{input}, std::vector{PacketQueuePtr(q16), PacketQueuePtr(q17), PacketQueuePtr(q18)}, long_processing));
    }

    void run() {
        for (auto& decoder : decoders) {
            tasks.emplace_back(std::move(decoder));
        }
    }

    void pushInput(Packet packet) {
        input.push(packet);
    }

    void waitForFinish() {
        for (auto& task : tasks) {
            task.Finish();
        }
    }

    Packet popOutput() {
        return output.pop();
    }

private:
    PacketQueue output;
    PacketQueue input;
    std::vector<std::unique_ptr<Decoder>> decoders;
    std::vector<std::unique_ptr<PacketQueue>> queues;
    std::vector<Task> tasks;
};