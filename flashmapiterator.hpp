#pragma once

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
template<typename VecRef, typename ValType>
class FlatHashMap<Key, Value, Hash>::IteratorBase {
    struct Proxy {
        const Key & first;
        ValType & second;

        Proxy * operator->() { return this; }
    };
    using ListVecIter = std::list<typename VecRef::iterator>::iterator;

public:
    IteratorBase(VecRef & data, ListVecIter pos, FlatHashMap & map) : m_Data(data), m_CurrentPosition(pos), m_Map(map) {}

    IteratorBase(const IteratorBase & other) : m_Data(other.m_Data), m_Map(other.m_Map) {
        m_Map.m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map.m_ActiveIterators.begin();
    }

    IteratorBase & operator=(const IteratorBase & other) {
        if (this == &other) return *this;
        m_Data = other.m_Data;
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
        auto it = *(m_CurrentPosition);
        return it->kv;
    }

    const auto & operator*() const {
        auto it = *m_CurrentPosition;
        return it->kv;
    }

    auto operator->() {
        auto it = *m_CurrentPosition;
        return Proxy{it->kv.key, it->kv.value};
    }

    auto operator->() const {
        auto it = *m_CurrentPosition;
        return Proxy{it->kv.key, it->kv.value};
    }

    bool operator==(const IteratorBase & other) const {
        return *m_CurrentPosition == *other.m_CurrentPosition;
    }

    bool operator!=(const IteratorBase & other) const {
        return *m_CurrentPosition != *other.m_CurrentPosition;
    }

private:
    void skipToOccupied() {
        auto newPos = *m_CurrentPosition;
        if (newPos == m_Data.end()) return;
        do {
            ++newPos;
        } while (newPos != m_Data.end() && newPos->status != Status::OCCUPIED);
        *m_CurrentPosition = newPos;
    }

    VecRef & m_Data;
    ListVecIter m_CurrentPosition;
    FlatHashMap & m_Map;
};