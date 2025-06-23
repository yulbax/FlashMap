#pragma once

namespace concepts {
    template<typename IteratorType, typename Key, typename Value, typename Hash>
    concept isIterator = std::same_as<IteratorType, typename flashmap<Key, Value, Hash>::iterator>
                      || std::same_as<IteratorType, typename flashmap<Key, Value, Hash>::const_iterator>;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
class flashmap<Key, Value, Hash>::iterator {
    struct Proxy {
        const Key & first;
        Value & second;

        Proxy * operator->() { return this; }
    };
    using ListIterator = std::list<std::size_t>::iterator;

public:

    using iterator_category = std::forward_iterator_tag;
    using value_type = KeyValue;
    using difference_type = std::ptrdiff_t;
    using pointer = Proxy*;
    using reference = KeyValue&;


    iterator(ListIterator pos, flashmap * map) : m_Map(map), m_CurrentPosition(pos) {}

    iterator(const iterator & other) {
        if (other.m_Map == nullptr) return;
        m_Map = other.m_Map;
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
    }

    explicit iterator(const const_iterator & other) : m_Map(other.m_Map) {
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
        auto & test = *m_Map;
        test.m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = test.m_ActiveIterators.begin();
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

    auto & operator*() {
        isAlive();
        auto & cell = m_Map->m_Data[*m_CurrentPosition];
        return cell.kv;
    }

    const auto & operator*() const {
        isAlive();
        auto & cell = m_Map->m_Data[*m_CurrentPosition];
        return cell.kv;
    }

    auto operator->() {
        isAlive();
        auto & cell = m_Map->m_Data[*m_CurrentPosition];
        return Proxy{cell.kv.key, cell.kv.value};
    }

    auto operator->() const {
        isAlive();
        auto & cell = m_Map->m_Data[*m_CurrentPosition];
        return Proxy{cell.kv.key, cell.kv.value};
    }

    template<typename Iterator> requires yulbax::concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator==(const Iterator & other) const {
        if (*m_CurrentPosition != m_Map->m_Data.size()) isAlive();
        return *m_CurrentPosition == *other.m_CurrentPosition;
    }

    template<typename Iterator> requires yulbax::concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator!=(const Iterator & other) const {
        if (*m_CurrentPosition != m_Map->m_Data.size()) isAlive();
        return *m_CurrentPosition != *other.m_CurrentPosition;
    }

private:
    void isAlive() const {
        if (m_Map->m_Data[*m_CurrentPosition].status == Status::DELETED) {
            throw std::out_of_range("Attempted to access a deleted value");
        }
    }

    void skipToOccupied() {
        if (*m_CurrentPosition == m_Map->m_Data.size()) return;
        do {
            ++(*m_CurrentPosition);
        } while (*m_CurrentPosition != m_Map->m_Data.size()
              && m_Map->m_Data[*m_CurrentPosition].status != Status::OCCUPIED);
    }

    flashmap * m_Map;
    ListIterator m_CurrentPosition;
    friend class const_iterator;
};

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
class flashmap<Key, Value, Hash>::const_iterator {
    struct Proxy {
        const Key & first;
        const Value & second;

        Proxy * operator->() { return this; }
    };
    using ListIterator = std::list<std::size_t>::iterator;

public:

    using iterator_category = std::forward_iterator_tag;
    using value_type = KeyValue;
    using difference_type = std::ptrdiff_t;
    using pointer = Proxy*;
    using reference = KeyValue&;


    const_iterator(ListIterator pos, const flashmap * map) : m_Map(map), m_CurrentPosition(pos) {}

    const_iterator(const const_iterator & other) {
        if (other.m_Map == nullptr) return;
        m_Map = other.m_Map;
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
    }

    explicit const_iterator(const iterator & other) : m_Map(other.m_Map) {
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
    }

    const_iterator operator=(const const_iterator &) = delete;

    ~const_iterator() {
        if (m_Map) {
            m_Map->m_ActiveIterators.erase(m_CurrentPosition);
        }
    }

    const_iterator & operator++() {
        skipToOccupied();
        return *this;
    }

    auto & operator*() {
        isAlive();
        auto & cell = m_Map->m_Data[*m_CurrentPosition];
        return cell.kv;
    }

    const auto & operator*() const {
        isAlive();
        auto & cell = m_Map->m_Data[*m_CurrentPosition];
        return cell.kv;
    }

    auto operator->() {
        isAlive();
        auto & cell = m_Map->m_Data[*m_CurrentPosition];
        return Proxy{cell.kv.key, cell.kv.value};
    }

    auto operator->() const {
        isAlive();
        auto & cell = m_Map->m_Data[*m_CurrentPosition];
        return Proxy{cell.kv.key, cell.kv.value};
    }

    template<typename Iterator> requires yulbax::concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator==(const Iterator & other) const {
        if (*m_CurrentPosition != m_Map->m_Data.size()) isAlive();
        return *m_CurrentPosition == *other.m_CurrentPosition;
    }

    template<typename Iterator> requires yulbax::concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator!=(const Iterator & other) const {
        if (*m_CurrentPosition != m_Map->m_Data.size()) isAlive();
        return *m_CurrentPosition != *other.m_CurrentPosition;
    }

private:
    void isAlive() const {
        if (m_Map->m_Data[*m_CurrentPosition].status == Status::DELETED) {
            throw std::out_of_range("Attempted to access a deleted value");
        }
    }

    void skipToOccupied() {
        if (*m_CurrentPosition == m_Map->m_Data.size()) return;
        do {
            ++(*m_CurrentPosition);
        } while (*m_CurrentPosition != m_Map->m_Data.size()
              && m_Map->m_Data[*m_CurrentPosition].status != Status::OCCUPIED);
    }

    const flashmap * m_Map;
    ListIterator m_CurrentPosition;
    friend class iterator;
};