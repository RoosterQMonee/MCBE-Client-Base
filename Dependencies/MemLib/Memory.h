#pragma once

#include <vector>
#include <stdexcept>
#include <memory>


namespace MemLib {
    // Safe(-ish) aligned memory helper class
    class MemoryManager {
    private:
        struct MemoryBlock {
            void* ptr;
            size_t size;
            size_t alignment;
        };

        std::vector<MemoryBlock> blocks_;

    public:
        MemoryManager() {};

        void* allocate(size_t size, size_t alignment) {
            void* ptr = _aligned_malloc(size, alignment);
            if (!ptr) throw std::bad_alloc();

            blocks_.push_back({ ptr, size, alignment });
            return ptr;
        }

        void free(void* ptr) {
            for (auto it = blocks_.begin(); it != blocks_.end(); ++it) {
                if (it->ptr == ptr) {
                    _aligned_free(ptr);
                    blocks_.erase(it);
                    return;
                }
            }
#ifndef NO_THROW_CRITICAL
            throw std::invalid_argument("Memory not found");
#endif
        }

        bool is_valid_memory(void* ptr) {
            MEMORY_BASIC_INFORMATION mbi;
            return VirtualQuery(ptr, &mbi, sizeof(mbi)) == sizeof(mbi) &&
                mbi.State == MEM_COMMIT &&
                mbi.Protect != PAGE_NOACCESS;
        }
    };

    static MemoryManager memoryManager = MemoryManager();
};