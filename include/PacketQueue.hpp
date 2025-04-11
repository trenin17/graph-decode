#include <queue>

#include "RingBufferQueue.hpp"

class PacketQueue {
public:

    PacketQueue(size_t mem_pool_size) : pool(mem_pool_size) {}

    void flush_page() {
        std::unique_lock<std::mutex> lock(mt);
        queue.push(std::move(push_page));
        lock.unlock();
        queue_not_empty.notify_one();
    }

    void flush_page(MemoryPagePtr&& page) {
        std::unique_lock<std::mutex> lock(mt);
        queue.push(std::move(page));
        lock.unlock();
        queue_not_empty.notify_one();
    }

    MemoryPagePtr get_new_page() {
        return MemoryPagePtr(&pool);
    }

    void push(const BlobPacket& packet) {
        std::unique_lock<std::mutex> lock(push_mt);
        if (!(push_page == nullptr) && push_page.full()) {
            flush_page();
        }
        if (push_page == nullptr) {
            push_page = MemoryPagePtr(&pool);
        }
        push_page.push(packet);
        if (packet.is_eof) {
            flush_page();
        }
    }

    BlobPacket pop() {
        // std::cout << "Pop" << std::endl;
        if (pop_queue.empty()) {
            std::unique_lock<std::mutex> lock(mt);
            while (queue.empty()) {
                queue_not_empty.wait(lock);    
            }
            pop_page = std::move(queue.front());
            queue.pop();
            lock.unlock();
            for (auto& packet : pop_page) {
                pop_queue.push(&packet);
            }
        }

        BlobPacket* packet = pop_queue.front();
        pop_queue.pop();
        return *packet;
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(mt);
        return queue.empty();
    }

    void add_producers(size_t n) {
        n_producers += n;
        // std::cout << n_producers << std::endl;
    }

    size_t get_producers() {
        return n_producers;
    }

private:
    MemoryPool pool;
    MemoryPagePtr push_page, pop_page;
    RingBufferQueue<BlobPacket*, MEMORY_PAGE_SIZE> pop_queue;

    std::mutex push_mt;
    size_t n_producers = 0;

    // Size doesn't sync with mem_pool_size
    RingBufferQueue<MemoryPagePtr, 16> queue;
    std::mutex mt;
    std::condition_variable queue_not_empty;
};

struct PacketQueuePtr {
    PacketQueuePtr() = default;
    PacketQueuePtr(PacketQueue& queue) : queue(&queue) {}

    void push(const BlobPacket& packet) {
        queue->push(packet);
    }

    void flush_page(MemoryPagePtr&& page) {
        queue->flush_page(std::move(page));
    }

    MemoryPagePtr get_new_page() {
        return queue->get_new_page();
    }

    BlobPacket pop() {
        return queue->pop();
    }

    void add_producers(size_t n) {
        queue->add_producers(n);
    }

    size_t get_producers() {
        return queue->get_producers();
    }

private:
    PacketQueue* queue = nullptr;   
};
