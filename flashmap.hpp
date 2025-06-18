#pragma once
#include <list>
#include <vector>
#include "flashmapimpl.hpp"

template<typename Key, typename HashFunc>
concept Hashable = requires(Key key, HashFunc hasher)
{ { hasher(key) } -> std::convertible_to<std::size_t>; };


template<typename Key,
         typename Value,
         typename Hash = std::hash<Key>>
         requires Hashable<Key, Hash>

class FlatHashMap {

    static constexpr std::size_t DEFAULT_SIZE = 1024;
    static constexpr float LOAD_FACTOR = 0.875;

    using Status = FlatHashMapImpl::Status;
    using KeyValue = FlatHashMapImpl::KeyValue<Key, Value>;
    using Element = FlatHashMapImpl::Element<Key, Value>;
    using Vec = std::vector<Element>;
    using VecIterator = Vec::iterator;

    template<typename VecType, typename ValType>
    class IteratorBase;

public:

    using key_type = Key;
    using value_type = Value;
    using iterator = IteratorBase<std::vector<Element>, Value>;
    using const_iterator = IteratorBase<std::vector<Element>, const Value>;

    explicit FlatHashMap(std::size_t size = DEFAULT_SIZE);

    FlatHashMap(const FlatHashMap & other);
    FlatHashMap & operator=(const FlatHashMap & other);

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

    decltype(auto) findIndex(const Key & key);

    auto getNextPosition(const Key & key);

    void killIterator(std::list<typename Vec::iterator>::iterator iter) const;

    void loadFactor() const;

    Vec m_Data;
    Hash m_Hasher;
    std::size_t m_Count;
    mutable std::size_t m_MaxLoad;
    mutable std::list<VecIterator> m_ActiveIterators;
};

#include "flashmapiterator.hpp"
#include "flashmap.tpp"
