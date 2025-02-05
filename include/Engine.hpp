#include "Task.hpp"
#include <optional>
#include <memory>
#include <tbb/flow_graph.h>

class Engine {
public:
    void initAndRun() {
        auto process_to_output = [this](Packet packet) {
            out_result.push(packet);
        };
        output_decoder = std::make_unique<Decoder>(g, process_to_output);


        decoders.push_back(std::make_unique<Decoder>(g));
        decoders[0]->MakeEdge(output_decoder);
        decoders.push_back(std::make_unique<Decoder>(g));
        decoders[1]->MakeEdge(output_decoder);
        decoders.push_back(std::make_unique<Decoder>(g));
        decoders[2]->MakeEdge(output_decoder);

        constexpr int n_layers = 6;
        for (int i = 0; i < n_layers - 1; i++) {
            decoders.push_back(std::make_unique<Decoder>(g));
            decoders[3 + 3 * i]->MakeEdge(decoders[3 * i]);
            decoders.push_back(std::make_unique<Decoder>(g));
            decoders[3 + 3 * i + 1]->MakeEdge(decoders[3 * i + 1]);
            decoders.push_back(std::make_unique<Decoder>(g));
            decoders[3 + 3 * i + 2]->MakeEdge(decoders[3 * i + 2]);
        }

        auto long_processing = [](Packet packet) {
            double packet_data = packet.data;
            for (int i = 0; i < 1000; i++) {
                packet_data = 10 + std::sqrt(packet_data + 10);
            }
        };
        input_decoder = std::make_unique<Decoder>(g, long_processing);
        input_decoder->MakeEdge(decoders[3 * n_layers - 1]);
        input_decoder->MakeEdge(decoders[3 * n_layers - 2]);
        input_decoder->MakeEdge(decoders[3 * n_layers - 3]);
    }

    void pushInput(Packet packet) {
        input_decoder->PushInput(packet);
    }

    void waitForFinish() {
        g.wait_for_all();
    }

    Packet popOutput() {
        auto res = out_result.front();
        out_result.pop();
        return res;
    }

private:
    std::vector<std::unique_ptr<Decoder>> decoders;
    std::vector<Task> tasks;

    std::unique_ptr<Decoder> input_decoder, output_decoder;

    std::queue<Packet> out_result;

    oneapi::tbb::flow::graph g;
};