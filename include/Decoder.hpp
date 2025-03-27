#include "PacketQueue.hpp"
#include <functional>

class Decoder {
public:
    Decoder(std::string name, PacketQueuePtr incoming, std::vector<PacketQueuePtr> outgoing,
            std::function<void(BlobPacket)> func = [](BlobPacket){}) : incoming(incoming), outgoing(outgoing), name(name), processPacket(func) {
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
            SendPacket(std::move(packet));
            if (eof) {
                n_eofs++;
                if (n_eofs == incoming.get_producers()) {
                    break;
                }
            }
        }
    }

    void SendPacket(BlobPacket&& packet) {
        packet.prev_decoder = name;
        //std::cout << "Sending packet " << packet.data << " to " << outgoing.size() << " queues" << std::endl;
        for (auto& queue : outgoing) {
            queue.push(packet);
        }
    }

private:
    PacketQueuePtr incoming;
    std::vector<PacketQueuePtr> outgoing;
    std::function<void(BlobPacket)> processPacket;
    size_t n_eofs = 0;
    std::string name;
};