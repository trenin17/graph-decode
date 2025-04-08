#include <mutex>
#include <condition_variable>

#include <queue>
#include <array>

const size_t PACKET_DATA_SIZE = 119;

#pragma pack(push, 1)
class alignas(128) BlobPacket{
public:
    bool is_eof = false;
    char data[PACKET_DATA_SIZE];
    size_t prev_decoder = 0;
};
#pragma pack(pop)

class MemoryPagePtr;

class MemoryPage{
public:
    void push(const BlobPacket& packet){
        if(next_packet < packets.size()){
            packets[next_packet++] = packet;
        }
    }
    
private:
    size_t next_packet = 0;
    std::array<BlobPacket, 16> packets;
    friend class MemoryPagePtr;
};

class MemoryPool{
public:
    MemoryPool(size_t size = 100){
        pages.resize(size);
        for(size_t i = 0; i < size; i++){
            free_pages.push(i);
        }
    }

private:
    MemoryPagePtr allocate();

    void free_page(size_t id) {
        std::lock_guard<std::mutex> lock(mt);
        free_pages.push(id);
        page_available.notify_one();
    }

    std::queue<size_t> free_pages;
    std::vector<MemoryPage> pages;

    std::mutex mt;
    std::condition_variable page_available;

    friend class MemoryPagePtr;
};

class MemoryPagePtr {
public:
    MemoryPagePtr() = default;

    MemoryPagePtr(MemoryPool* pool) : pool(pool) {
        if (pool) {
            *this = pool->allocate();
        }
    }

    MemoryPagePtr(MemoryPagePtr&& other) {
        page = other.page;
        id = other.id;
        pool = other.pool;
        other.page = nullptr;
        other.pool = nullptr;
    }

    MemoryPagePtr& operator=(MemoryPagePtr&& other) {
        free();
        page = other.page;
        id = other.id;
        pool = other.pool;
        other.page = nullptr;
        other.pool = nullptr;
        return *this;
    }

    MemoryPagePtr(const MemoryPagePtr&) = delete;
    MemoryPagePtr& operator=(const MemoryPagePtr&) = delete;

    void free(){
        if(!page || !pool){
            return;
        }
        page->next_packet = 0;
        pool->free_page(id);
        page = nullptr;
    }

    BlobPacket& operator[](size_t index){
        return page->packets[index];
    }

    auto begin(){
        return page->packets.begin();
    }

    auto end(){
        return page->packets.begin() + page->next_packet;
    }

    bool full(){
        return page && page->next_packet == page->packets.size();
    }

    void push(const BlobPacket& packet){
        return page->push(packet);
    }

    size_t next_packet() {
        return page->next_packet;
    }

    ~MemoryPagePtr(){
        free();
    }

private:
    MemoryPagePtr(MemoryPage* page, size_t id, MemoryPool* pool) : page(page), id(id), pool(pool) {}

    MemoryPage* page = nullptr;
    size_t id = 0;
    MemoryPool* pool = nullptr;

    friend class MemoryPool;
    friend bool operator==(const MemoryPagePtr& ptr, std::nullptr_t);
    friend bool operator==(std::nullptr_t, const MemoryPagePtr& ptr);
};

bool operator==(const MemoryPagePtr& ptr, std::nullptr_t) {
    return ptr.page == nullptr;
}

bool operator==(std::nullptr_t, const MemoryPagePtr& ptr) {
    return ptr.page == nullptr;
}

MemoryPagePtr MemoryPool::allocate(){
    std::unique_lock<std::mutex> lock(mt);
    page_available.wait(lock, [this](){ 
            return !free_pages.empty(); });
    // std::cout << "Page ready" << std::endl;

    size_t id = free_pages.front();
    free_pages.pop();
    return MemoryPagePtr{&pages[id], id, this};
}
