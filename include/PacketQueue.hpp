#include <queue>
#include <mutex>
#include <condition_variable>

struct Packet {
    int data;
    bool eof = false;
};

class PacketQueue {
public:

    void push(Packet packet) {
        std::unique_lock<std::mutex> lock(mt);
        queue.push(packet);
        lock.unlock();
        condition.notify_one();
    }

    Packet pop() {
        //std::cout << "Pop" << std::endl;
        std::unique_lock<std::mutex> lock(mt);
        while (queue.empty()) {
            //std::cout << "Empty" << std::endl;
            condition.wait(lock);
        }
        Packet packet = queue.front();
        queue.pop();
        //std::cout << "Return pop " << packet.data << std::endl;
        return packet;
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(mt);
        return queue.empty();
    }

private:
    std::queue<Packet> queue;
    std::mutex mt;
    std::condition_variable condition;
};

struct PacketQueuePtr {
    PacketQueuePtr(PacketQueue& queue) : queue(&queue) {}

    void push(const Packet& packet) {
        queue->push(packet);
    }

    Packet pop() {
        return queue->pop();
    }

private:
    PacketQueue* queue;   
};
