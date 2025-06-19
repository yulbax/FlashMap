#pragma once
#include <list>
#include <vector>
#include "flashmapconcepts.hpp"
#include "flashmapimpl.hpp"

template<typename Key,
         typename Value,
         typename Hash = std::hash<Key>>
         requires Hashable<Key, Hash>

class FlashMap {

    static constexpr std::size_t DEFAULT_SIZE = 1024;
    static constexpr float LOAD_FACTOR = 0.875;

    using Status = FlashMapImpl::Status;
    using KeyValue = FlashMapImpl::KeyValue<Key, Value>;
    using Element = FlashMapImpl::Element<Key, Value>;
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

    explicit FlashMap(std::size_t size = DEFAULT_SIZE);

    FlashMap(const FlashMap & other);
    FlashMap & operator=(const FlashMap & other);

    template<typename InputIt> requires InputPairs<InputIt, Key, Value>
    FlashMap(InputIt first, InputIt last);

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

    void loadFactor() const;

    Vec m_Data;
    Hash m_Hasher;
    std::size_t m_Count;
    mutable std::size_t m_MaxLoad;
    mutable std::list<VecIterator> m_ActiveIterators;
};

#include "flashmapiterator.hpp"
#include "flashmap.tpp"
