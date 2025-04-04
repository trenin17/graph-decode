#include "PacketQueue.hpp"
#include <functional>

class Decoder {
public:
    Decoder(size_t name, PacketQueuePtr incoming, std::vector<PacketQueuePtr> outgoing,
            std::function<void(const BlobPacket&)> func = [](const BlobPacket&){}) : incoming(incoming), outgoing(outgoing), id(name), processPacket(func) {
    }

    void AddOutgoing(PacketQueuePtr queue) {
        outgoing.push_back(queue);
    }

    void AddPacketHandler(std::function<void(const BlobPacket&)> func) {
        processPacket = func;
    }

    PacketQueuePtr GetIncoming() {
        return incoming;
    }

    void init() {
        for (auto& queue : outgoing) {
            queue.add_producers(incoming.get_producers());
        }
    }

    void Run() {
        //std::cout << "Running decoder" << std::endl;
        while (true) {
            BlobPacket packet = incoming.pop();
            bool eof = packet.is_eof;
            processPacket(packet);
            SendPacket(packet);
            if (eof) {
                n_eofs++;
                if (n_eofs == incoming.get_producers()) {
                    break;
                }
            }
        }
    }

    void SendPacket(BlobPacket& packet) {
        packet.prev_decoder = id;
        //std::cout << "Sending packet " << packet.data << " to " << outgoing.size() << " queues" << std::endl;
        for (auto& queue : outgoing) {
            queue.push(packet);
        }
    }

private:
    PacketQueuePtr incoming;
    std::vector<PacketQueuePtr> outgoing;
    std::function<void(const BlobPacket&)> processPacket;
    size_t n_eofs = 0;
    size_t id = 0;
};