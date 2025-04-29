#include "PacketQueue.hpp"
#include <functional>
#include <Windows.h>
#include <processthreadsapi.h>
#include <winbase.h>


void bind_to_core(DWORD core_id)
{
    HANDLE thread           = GetCurrentThread();  // Get handle to the current thread
    DWORD_PTR affinity_mask = 1ll << core_id;      // Create a bitmask with the desired core
    SetThreadAffinityMask(thread, affinity_mask);  // Bind thread to core
}

class Decoder {
public:
    Decoder(size_t name, PacketQueuePtr incoming, std::vector<PacketQueuePtr> outgoing,
            std::function<void(const BlobPacket&)> func = [](const BlobPacket&){}) : incoming(incoming), outgoing(outgoing), id(name), processPacket(func) {
        outgoing_pages.resize(outgoing.size());
    }

    void AddOutgoing(PacketQueuePtr queue) {
        outgoing.push_back(queue);
        outgoing_pages.push_back(MemoryPagePtr{});
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
        // bind_to_core(id);
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

    void SendPacketToQueue(size_t queue_id, const BlobPacket& packet) {
        auto& queue = outgoing[queue_id];
        auto& push_page = outgoing_pages[queue_id];
        if (!(push_page == nullptr) && push_page.full()) {
            queue.flush_page(std::move(push_page));
        }
        if (push_page == nullptr) {
            push_page = queue.get_new_page();
        }
        push_page.push(packet);
        if (packet.is_eof) {
            queue.flush_page(std::move(push_page));
        }
    }

    void SendPacket(BlobPacket& packet) {
        packet.prev_decoder = id;

        for (size_t queue_id = 0; queue_id < outgoing.size(); queue_id++) {
            SendPacketToQueue(queue_id, packet);
        }
    }

private:
    PacketQueuePtr incoming;
    std::vector<PacketQueuePtr> outgoing;
    std::vector<MemoryPagePtr> outgoing_pages;
    std::function<void(const BlobPacket&)> processPacket;
    size_t n_eofs = 0;
    size_t id = 0;
    friend class Task;
};