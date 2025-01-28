#include "PacketQueue.hpp"
#include <functional>

class Decoder {
public:
    Decoder(PacketQueuePtr incoming, std::vector<PacketQueuePtr> outgoing,
            std::function<void(Packet)> func = [](Packet){}) : incoming(incoming), outgoing(outgoing), processPacket(func) {}

    void Run() {
        //std::cout << "Running decoder" << std::endl;
        while (true) {
            Packet packet = incoming.pop();
            bool eof = packet.eof;
            processPacket(packet);
            SendPacket(std::move(packet));
            if (eof) {
                break;
            }
        }
    }

    void SendPacket(Packet&& packet) {
        //std::cout << "Sending packet " << packet.data << " to " << outgoing.size() << " queues" << std::endl;
        for (auto& queue : outgoing) {
            queue.push(packet);
        }
    }

private:
    PacketQueuePtr incoming;
    std::vector<PacketQueuePtr> outgoing;
    std::function<void(Packet)> processPacket;
};