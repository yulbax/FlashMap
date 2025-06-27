#pragma once
#include <cstddef>
#include <vector>
#include <cassert>

namespace yulbax::container::allocator {
    template <typename T>
    class chunk_list_allocator {
    public:
        using value_type = T;

        static constexpr std::size_t CHUNK_SIZE = 512;

        chunk_list_allocator() : freeMem(nullptr), currentChunk(nullptr), usedInChunk(0) {
            chunks.reserve(10);
        }

        chunk_list_allocator(const chunk_list_allocator&) noexcept : freeMem(nullptr), currentChunk(nullptr), usedInChunk(0) {}

        template <typename U>
        explicit chunk_list_allocator(const chunk_list_allocator<U>&) noexcept : freeMem(nullptr), currentChunk(nullptr), usedInChunk(0) {}

        ~chunk_list_allocator() {
            for (auto * chunk : chunks) {
                ::operator delete(chunk);
            }
        }

        T * allocate(std::size_t n) {
            if (freeMem) {
                T * result = freeMem;
                freeMem = *reinterpret_cast<T**>(freeMem);
                return result;
            }

            if (currentChunk == nullptr || usedInChunk == CHUNK_SIZE) {
                T * new_chunk = static_cast<T*>(::operator new(sizeof(T) * CHUNK_SIZE));
                chunks.emplace_back(new_chunk);
                currentChunk = new_chunk;
                usedInChunk = 0;
            }

            T * result = currentChunk + usedInChunk;
            ++usedInChunk;
            return result;
        }

        void deallocate(T * p, std::size_t n) noexcept {
            *reinterpret_cast<T**>(p) = freeMem;
            freeMem = p;
        }

        template <typename U, typename... Args>
        void construct(U * p, Args&&... args) {
            ::new (p) U(std::forward<Args>(args)...);
        }

        template <typename U>
        static void destroy(U * p) noexcept {
            p->~U();
        }

    private:
        T * freeMem;
        T * currentChunk;
        std::size_t usedInChunk;
        std::vector<T*> chunks;
    };
}
