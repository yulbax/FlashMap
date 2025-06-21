#pragma once
#include <list>
#include <vector>
#include "flashmapconcepts.hpp"

namespace yulbax {

    template<typename Key,
             typename Value,
             typename Hash = std::hash<Key>>
             requires concepts::hashable<Key, Hash>

    class flashmap {

        static constexpr std::size_t DEFAULT_SIZE = 1024;
        static constexpr float LOAD_FACTOR = 0.875;

        enum class Status;
        class KeyValue;
        class Element;
        using Vec = std::vector<Element>;
        using VecIterator = typename Vec::iterator;
        using ListIterator = typename std::list<VecIterator>::iterator;

        template<typename VecType, typename ValType>
        class IteratorBase;

    public:

        using key_type = Key;
        using mapped_type = Value;
        using value_type = std::pair<const Key, Value>;
        using iterator = IteratorBase<std::vector<Element>, Value>;
        using const_iterator = IteratorBase<std::vector<Element>, const Value>;

        explicit flashmap(std::size_t size = DEFAULT_SIZE);

        flashmap(const flashmap & other);
        flashmap & operator=(const flashmap & other);

        template<typename InputIt> requires concepts::inititerator<InputIt, Key, Value>
        flashmap(InputIt first, InputIt last);

        template<typename K, typename V>
        bool insert(K && key, V && value);

        template<typename K>
        Value & operator[](K && key);

        Value & at(const Key & key);
        [[nodiscard]] const Value & at(const Key & key) const;

        [[nodiscard]] bool contains(const Key & key) const;

        [[nodiscard]] std::size_t size() const;

        bool erase(const Key & key);

        void clear();

        iterator find(const Key & key);
        [[nodiscard]] const_iterator find(const Key & key) const;

        iterator begin();
        [[nodiscard]] const_iterator begin() const;

        iterator end();
        [[nodiscard]] const_iterator end() const;

    private:
        [[nodiscard]] std::size_t hash(const Key & key) const;

        void rehash();

        [[nodiscard]] std::size_t nextCell(std::size_t index, std::size_t shift) const;

        auto findIndex(const Key & key);
        [[nodiscard]] auto findIndex(const Key & key) const;

        auto getNextPosition(const Key & key);

        void killIterator(ListIterator iter) const;

        void loadFactor();

        Vec m_Data;
        Hash m_Hasher;
        std::size_t m_Count;
        std::size_t m_MaxLoad;
        mutable std::list<VecIterator> m_ActiveIterators;
    };

    #include "flashmapimpl.hpp"
    #include "flashmapiterator.hpp"
    #include "flashmap.tpp"

}

namespace std {
    template<typename Key, typename Value>
    struct tuple_size<typename yulbax::flashmap<Key, Value>::template KeyValue<Key, Value>> { // NOLINT(*-dcl58-cpp)
        static constexpr std::size_t value = 2;
    };

    template<std::size_t I, typename Key, typename Value>
    struct tuple_element<I, typename yulbax::flashmap<Key, Value>::template KeyValue<Key, Value>> { // NOLINT(*-dcl58-cpp)
        using type = std::conditional_t<I == 0, const Key, Value>;
    };

    template<std::size_t I, typename Key, typename Value>
    struct tuple_element<I, const typename yulbax::flashmap<Key, Value>::template KeyValue<Key, Value>> { // NOLINT(*-dcl58-cpp)
        using type = std::conditional_t<I == 0, const Key, const Value>;
    };
}
