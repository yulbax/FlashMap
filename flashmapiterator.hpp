#pragma once

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
template<typename VecRef, typename ValType>
class flashmap<Key, Value, Hash>::IteratorBase {
    struct Proxy {
        const Key & first;
        ValType & second;

        Proxy * operator->() { return this; }
    };
    using ListVecIter = typename std::list<typename VecRef::iterator>::iterator;

public:

    using iterator_category = std::forward_iterator_tag;
    using value_type = KeyValue;
    using difference_type = std::ptrdiff_t;
    using pointer = Proxy*;
    using reference = KeyValue&;


    IteratorBase(ListVecIter pos, flashmap & map) : m_CurrentPosition(pos), m_Map(map) {}

    IteratorBase(const IteratorBase & other) : m_Map(other.m_Map) {
        m_Map.m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map.m_ActiveIterators.begin();
    }

    IteratorBase & operator=(const IteratorBase & other) {
        if (this == &other) return *this;
        m_Map = other.m_Map;
        m_Map.m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map.m_ActiveIterators.begin();
        return *this;
    }

    ~IteratorBase() {
        m_Map.killIterator(m_CurrentPosition);
    }

    IteratorBase & operator++() {
        skipToOccupied();
        return *this;
    }

    auto & operator*() {
        isAlive();
        auto it = *m_CurrentPosition;
        return it->kv;
    }

    const auto & operator*() const {
        isAlive();
        auto it = *m_CurrentPosition;
        return it->kv;
    }

    auto operator->() {
        isAlive();
        auto it = *m_CurrentPosition;
        return Proxy{it->kv.key, it->kv.value};
    }

    auto operator->() const {
        isAlive();
        auto it = *m_CurrentPosition;
        return Proxy{it->kv.key, it->kv.value};
    }

    bool operator==(const IteratorBase & other) const {
        if (*m_CurrentPosition != m_Map.m_Data.end()) isAlive();
        return *m_CurrentPosition == *other.m_CurrentPosition;
    }

    bool operator!=(const IteratorBase & other) const {
        if (*m_CurrentPosition != m_Map.m_Data.end()) isAlive();
        return *m_CurrentPosition != *other.m_CurrentPosition;
    }

private:
    void isAlive() const {
        if ((*m_CurrentPosition)->status == Status::DELETED) {
            throw std::out_of_range("Attempted to access a deleted value");
        }
    }

    void skipToOccupied() {
        auto newPos = *m_CurrentPosition;
        if (newPos == m_Map.m_Data.end()) return;
        do {
            ++newPos;
        } while (newPos != m_Map.m_Data.end() && newPos->status != Status::OCCUPIED);
        *m_CurrentPosition = newPos;
    }

    ListVecIter m_CurrentPosition;
    flashmap & m_Map;
};