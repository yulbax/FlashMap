#pragma once

namespace concepts {
    template<typename IteratorType, typename Key, typename Value, typename Hash>
    concept isIterator = std::same_as<IteratorType, typename flashmap<Key, Value, Hash>::iterator>
                      || std::same_as<IteratorType, typename flashmap<Key, Value, Hash>::const_iterator>;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
class flashmap<Key, Value, Hash>::iterator {

    using ListIterator = std::list<std::size_t>::iterator;

public:

    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const Key, Value>;
    using difference_type = std::ptrdiff_t;
    using pointer = std::pair<const Key, Value>*;
    using reference = value_type&;

    iterator(const ListIterator pos, flashmap * map) : m_Map(map), m_CurrentPosition(pos) {}

    iterator(const iterator & other) {
        if (other.m_Map == nullptr) return;
        m_Map = other.m_Map;
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
    }

    explicit iterator(const const_iterator & other) {
        if (other.m_Map == nullptr) return;
        m_Map = other.m_Map;
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
    }

    iterator(iterator && other) noexcept
        : m_Map(other.m_Map), m_CurrentPosition(other.m_CurrentPosition) {
        other.m_Map = nullptr;
    }

    iterator & operator=(const iterator & other) {
        if (this == &other) return *this;
        m_Map = other.m_Map;
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
        return *this;
    }

    iterator & operator=(iterator && other) noexcept {
        if (this == &other) return *this;
        m_Map = other.m_Map;
        m_CurrentPosition = other.m_CurrentPosition;
        other.m_Map = nullptr;
        return *this;
    }

    ~iterator() {
        if (m_Map) {
            m_Map->m_ActiveIterators.erase(m_CurrentPosition);
        }
    }

    iterator & operator++() {
        skipToOccupied();
        return *this;
    }

    std::pair<const Key, Value> & operator*() const {
        isAlive();
        return *std::launder(reinterpret_cast<std::pair<const Key, Value>*>(&m_Map->m_Data.KVs[*m_CurrentPosition]));
    }

    auto operator->() const {
        isAlive();
        return std::launder(reinterpret_cast<std::pair<const Key, Value>*>(&m_Map->m_Data.KVs[*m_CurrentPosition]));
    }

    template<typename Iterator> requires yulbax::concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator==(const Iterator & other) const {
        if (*m_CurrentPosition != m_Map->m_Data.size()) isAlive();
        return *m_CurrentPosition == *other.m_CurrentPosition;
    }

    template<typename Iterator> requires yulbax::concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator!=(const Iterator & other) const {
        return !(*this == other);
    }

private:
    void isAlive() const {
        if (m_Map->m_Data.statuses[*m_CurrentPosition] == Status::DELETED) {
            throw std::out_of_range("Attempted to access a deleted value");
        }
    }

    void skipToOccupied() {
        if (*m_CurrentPosition == m_Map->m_Data.size()) return;
        do {
            ++(*m_CurrentPosition);
        } while (*m_CurrentPosition != m_Map->m_Data.size()
              && m_Map->m_Data.statuses[*m_CurrentPosition] != Status::OCCUPIED);
    }

    flashmap * m_Map;
    ListIterator m_CurrentPosition;
    friend class const_iterator;
    friend class flashmap;
};

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
class flashmap<Key, Value, Hash>::const_iterator {

    using ListIterator = std::list<std::size_t>::iterator;

public:

    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const Key, const Value>;
    using difference_type = std::ptrdiff_t;
    using pointer = std::pair<const Key, const Value>*;
    using reference = value_type&;


    const_iterator(ListIterator pos, const flashmap * map) : m_Map(map), m_CurrentPosition(pos) {}

    const_iterator(const const_iterator & other) {
        if (other.m_Map == nullptr) return;
        m_Map = other.m_Map;
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
    }

    explicit const_iterator(const iterator & other) {
        if (other.m_Map == nullptr) return;
        m_Map = other.m_Map;
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
    }

    const_iterator & operator=(const const_iterator & other) {
        if (this == &other) return *this;
        m_Map = other.m_Map;
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
        return *this;
    }

    const_iterator & operator=(const iterator & other) {
        m_Map = other.m_Map;
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
        return *this;
    }

    ~const_iterator() {
        if (m_Map) {
            m_Map->m_ActiveIterators.erase(m_CurrentPosition);
        }
    }

    const_iterator & operator++() {
        skipToOccupied();
        return *this;
    }

    auto & operator*() const {
        isAlive();
        return *std::launder(reinterpret_cast<std::pair<const Key, const Value>*>(&m_Map->m_Data.KVs[*m_CurrentPosition]));
    }

    auto operator->() const {
        isAlive();
        return std::launder(reinterpret_cast<std::pair<const Key, const Value>*>(&m_Map->m_Data.KVs[*m_CurrentPosition]));
    }

    template<typename Iterator> requires yulbax::concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator==(const Iterator & other) const {
        if (*m_CurrentPosition != m_Map->m_Data.size()) isAlive();
        return *m_CurrentPosition == *other.m_CurrentPosition;
    }

    template<typename Iterator> requires yulbax::concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator!=(const Iterator & other) const {
        return !(*this == other);
    }

private:
    void isAlive() const {
        if (m_Map->m_Data.statuses[*m_CurrentPosition] == Status::DELETED) {
            throw std::out_of_range("Attempted to access a deleted value");
        }
    }

    void skipToOccupied() {
        if (*m_CurrentPosition == m_Map->m_Data.size()) return;
        do {
            ++(*m_CurrentPosition);
        } while (*m_CurrentPosition != m_Map->m_Data.size()
              && m_Map->m_Data.statuses[*m_CurrentPosition] != Status::OCCUPIED);
    }



    const flashmap * m_Map;
    ListIterator m_CurrentPosition;
    friend class iterator;
    friend class flashmap;
};