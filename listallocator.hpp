#pragma once
#include <cstddef>
#include <cassert>

namespace yulbax::container::allocator {
    class chunk_memory_pool {
    public:
        static constexpr std::size_t CHUNK_SIZE = 128;

        struct FreeNode {
            FreeNode * next;
        };

        template<typename T>
        struct Chunk {
            alignas(T) std::byte data[sizeof(T) * CHUNK_SIZE];
            Chunk * next;
        };

        template<typename T>
        T * allocate() {
            auto & state = get_state<T>();

            if (state.freeMem) {
                FreeNode * node = state.freeMem;
                state.freeMem = node->next;
                return reinterpret_cast<T*>(node);
            }

            if (!state.chunkList || state.usedInChunk == CHUNK_SIZE) {
                allocChunk<T>();
            }

            T * rawPtr = &reinterpret_cast<T*>(state.chunkList->data)[state.usedInChunk];
            ++state.usedInChunk;
            return rawPtr;
        }

        template<typename T>
        void deallocate(T * p) noexcept {
            auto & state = get_state<T>();
            auto node = reinterpret_cast<FreeNode*>(p);
            node->next = state.freeMem;
            state.freeMem = node;
        }

    private:
        template<typename T>
        struct TypeState {
            FreeNode * freeMem = nullptr;
            Chunk<T> * chunkList = nullptr;
            std::size_t usedInChunk = 0;

            ~TypeState() {
                while (chunkList) {
                    auto * next = chunkList->next;
                    delete chunkList;
                    chunkList = next;
                }
            }
        };

        template<typename T>
        TypeState<T> & get_state() {
            static TypeState<T> state;
            return state;
        }

        template<typename T>
        void allocChunk() {
            auto & state = get_state<T>();
            auto * newchunk = new Chunk<T>;
            newchunk->next = state.chunkList;
            state.chunkList = newchunk;
            state.usedInChunk = 0;
        }

        static chunk_memory_pool & instance() {
            static chunk_memory_pool pool;
            return pool;
        }

        template<typename T> friend class chunk_list_allocator;
    };

    template <typename T>
    class chunk_list_allocator {
    public:
        using value_type = T;

        chunk_list_allocator() = default;

        template <typename U>
        explicit chunk_list_allocator(const chunk_list_allocator<U>&) noexcept {}

        template <typename U>
        struct rebind {
            using other = chunk_list_allocator<U>;
        };

        static T * allocate(const std::size_t n) {
            if (n != 1) throw std::bad_alloc();
            return chunk_memory_pool::instance().allocate<T>();
        }

        static void deallocate(T * p, std::size_t n) noexcept {
            chunk_memory_pool::instance().deallocate(p);
        }

        template <typename U, typename... Args>
        void construct(U * p, Args&&... args) {
            ::new (p) U(std::forward<Args>(args)...);
        }

        template <typename U>
        static void destroy(U * p) noexcept {
            p->~U();
        }

        template<typename U>
        bool operator==(const chunk_list_allocator<U>&) const noexcept { return true; }
        template<typename U>
        bool operator!=(const chunk_list_allocator<U>&) const noexcept { return false; }

        using propagate_on_container_move_assignment = std::true_type;
        using is_always_equal = std::true_type;
    };
}
