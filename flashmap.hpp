#pragma once

#include <list>
#include <vector>
#include <memory>
#include <ranges>
#include "flashmapconcepts.hpp"
#include "flashmapimpl.hpp"
#include "listallocator.hpp"

namespace yulbax {

    template<typename Key,
             typename Value,
             typename Hash = std::hash<Key>>
             requires concepts::hashable<Key, Hash>

    class flashmap {

        static constexpr std::size_t DEFAULT_SIZE = 1024;
        static constexpr float LOAD_FACTOR = 0.875;

        using HashType = decltype(std::declval<Hash>()(std::declval<Key>()));
        using Status   = container::flashmap::impl::Status;
        using Data     = container::flashmap::impl::Vectors<Key, Value, HashType>;

        template<typename IteratorType, typename MapType>
        class IteratorBase;

    public:

        using key_type       = Key;
        using mapped_type    = Value;
        using value_type     = std::pair<const Key, Value>;
        using iterator       = IteratorBase<Value, flashmap>;
        using const_iterator = IteratorBase<const Value, const flashmap>;

        explicit flashmap(std::size_t size = DEFAULT_SIZE);

        flashmap(const flashmap & other);
        flashmap & operator=(const flashmap & other);
        flashmap(flashmap && other) noexcept;
        flashmap & operator=(flashmap && other) noexcept;

        template<typename InputIt> requires concepts::inititerator<InputIt, Key, Value>
        flashmap(InputIt first, InputIt last);

        template<typename K, typename V>
        bool insert(K && key, V && value);

        template<typename K, typename V>
        std::pair<iterator, bool> emplace(K && key, V && value);

        template<typename K>
        Value & operator[](K && key);

        Value & at(const Key & key);
        [[nodiscard]] const Value & at(const Key & key) const;

        [[nodiscard]] bool contains(const Key & key) const;

        [[nodiscard]] std::size_t size() const;

        bool erase(const Key & key);
        bool erase(iterator it);

        void clear();

        iterator find(const Key & key);
        [[nodiscard]] const_iterator find(const Key & key) const;

        iterator begin();
        [[nodiscard]] const_iterator begin() const;

        iterator & end();
        [[nodiscard]] const const_iterator & end() const;

    private:
        void rehash();

        [[nodiscard]] std::size_t nextCell(HashType hash, std::size_t shift) const;

        [[nodiscard]] std::size_t findIndex(const Key & key) const;

        std::size_t getNextPosition(const Key & key, HashType hash);

        [[nodiscard]] std::size_t loadFactor() const;

        Data m_Data;
        Hash m_Hasher;
        std::size_t m_Count;
        std::size_t m_MaxLoad;

        mutable std::list<std::size_t, container::allocator::chunk_list_allocator<size_t>> m_ActiveIterators;
        std::shared_ptr<bool> status;
        iterator endIt;
        const_iterator cendIt;
    };

    #include "flashmapiterator.hpp"
    #include "flashmap.tpp"
}