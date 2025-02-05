#include "PacketQueue.hpp"
#include <functional>
#include <tbb/flow_graph.h>

class Decoder {
public:
    Decoder(oneapi::tbb::flow::graph& graph, std::function<void(Packet)> func = [](Packet){}) :
        g(graph), 
        processPacket(func),
        node(g, oneapi::tbb::flow::serial, [this](Packet packet) {
            processPacket(packet);
            return packet;
        }) {}

    void MakeEdge(std::unique_ptr<Decoder>& decoder) {
        oneapi::tbb::flow::make_edge(node, decoder->node);
    }

    void PushInput(Packet packet) {
        node.try_put(packet);
    }

private:
    oneapi::tbb::flow::graph& g;
    oneapi::tbb::flow::function_node<Packet, Packet> node;
    std::function<void(Packet)> processPacket;
};