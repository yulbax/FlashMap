#pragma once

namespace concepts {
    template<typename IteratorType, typename Key, typename Value, typename Hash>
    concept isIterator = std::same_as<IteratorType, typename flashmap<Key, Value, Hash>::iterator>
                      || std::same_as<IteratorType, typename flashmap<Key, Value, Hash>::const_iterator>;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename ValueType, typename MapType>
class flashmap<Key, Value, Hash>::Iterator {
    using ListPos = typename std::list<IteratorPtr>::iterator;
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const Key, ValueType>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    Iterator() : m_Map(nullptr), m_Index() {}

    Iterator(MapType * map, const std::size_t index) : m_Map(map), m_Index(index) {
        if (m_Map) m_Map->registerIterator(this);
    }

    Iterator(const Iterator & other) : m_Index(other.m_Index) {
        m_Map = other.m_Map;
        if (m_Map) m_Map->registerIterator(this);
    }

    Iterator & operator=(const Iterator & other) {
        if (this == &other) return *this;

        if (m_Map) m_Map->unregisterIterator(this);
        m_Map = other.m_Map;
        m_Index = other.m_Index;
        if (m_Map) m_Map->registerIterator(this);

        return *this;
    }

    Iterator(Iterator && other) noexcept : m_Index(other.m_Index) {
        m_Map = other.m_Map;
        m_IteratorNode = other.m_IteratorNode;
        other.m_Map = nullptr;
        other.m_IteratorNode = {};
        if (m_Map) *m_IteratorNode = this;
    }

    Iterator& operator=(Iterator && other) noexcept {
        if (this == &other) return *this;

        if (m_Map) m_Map->unregisterIterator(this);
        m_Map = other.m_Map;
        m_Index = other.m_Index;
        m_IteratorNode = other.m_IteratorNode;
        if (m_Map) *m_IteratorNode = this;

        other.m_Map = nullptr;
        other.m_IteratorNode = {};

        return *this;
    }

    ~Iterator() {
        if (m_Map) m_Map->unregisterIterator(this);
    }

    Iterator & operator++() {
        skipToOccupied();
        return *this;
    }

    auto & operator*() const {
        isAlive();
        return *std::launder(reinterpret_cast<std::pair<const Key, Value>*>(&m_Map->m_Data.KVs[m_Index]));
    }

    auto operator->() const {
        isAlive();
        return std::launder(reinterpret_cast<std::pair<const Key, Value>*>(&m_Map->m_Data.KVs[m_Index]));
    }

    template<typename Iterator> requires concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator==(const Iterator & other) const {
        if (m_Map != other.m_Map || !m_Map) return false;
        if (other.m_Index != other.m_Map->m_Data.size()) other.isAlive();
        if (m_Index != m_Map->m_Data.size()) isAlive();
        return m_Index == other.m_Index;
    }

    template<typename Iterator> requires concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator!=(const Iterator & other) const {
        return !(*this == other);
    }

private:
    void isAlive() const {
        if (!m_Map) {
            throw std::runtime_error("Iterator invalidated: container was destroyed");
        }

        if (m_Map->m_Data.statuses[m_Index] == Status::DELETED) {
            throw std::out_of_range("Attempted to access a deleted value");
        }
    }

    void skipToOccupied() {
        do {
            ++m_Index;
        } while (m_Index != m_Map->m_Data.size()
              && m_Map->m_Data.statuses[m_Index] != Status::OCCUPIED);
    }

    MapType * m_Map;
    std::size_t m_Index;
    ListPos m_IteratorNode;

    friend class flashmap;
    friend class Iterator<Value, flashmap>;
    friend class Iterator<const Value, const flashmap>;
};
