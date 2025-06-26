#pragma once

namespace concepts {
    template<typename IteratorType, typename Key, typename Value, typename Hash>
    concept isIterator = std::same_as<IteratorType, typename flashmap<Key, Value, Hash>::iterator>
                      || std::same_as<IteratorType, typename flashmap<Key, Value, Hash>::const_iterator>;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename ValueType, typename MapType>
class flashmap<Key, Value, Hash>::IteratorBase {
    using ListIterator = std::list<std::size_t>::iterator;
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const Key, ValueType>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    IteratorBase() : m_Map(nullptr) {}

    IteratorBase(const ListIterator pos, MapType * map, const std::shared_ptr<bool> & status) : m_Map(map), m_CurrentPosition(pos), m_MapStatus(status) {}

    IteratorBase(const IteratorBase & other) {
        if (other.m_Map == nullptr) return;
        m_Map = other.m_Map;
        m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
        m_CurrentPosition = m_Map->m_ActiveIterators.begin();
        m_MapStatus = other.m_MapStatus;
    }

    IteratorBase & operator=(const IteratorBase & other) {
        if (this == &other) return *this;
        if (!m_MapStatus.expired()) m_Map->m_ActiveIterators.erase(m_CurrentPosition);
        m_Map = other.m_Map;
        if (!other.m_MapStatus.expired()) {
            m_Map->m_ActiveIterators.emplace_front(*other.m_CurrentPosition);
            m_CurrentPosition = m_Map->m_ActiveIterators.begin();
        }
        m_MapStatus = other.m_MapStatus;

        return *this;
    }

    IteratorBase(IteratorBase && other) noexcept
    : m_Map(other.m_Map), m_CurrentPosition(other.m_CurrentPosition), m_MapStatus(other.m_MapStatus) {
        other.m_MapStatus.reset();
    }

    IteratorBase & operator=(IteratorBase && other) noexcept {
        if (this == &other) return *this;
        m_Map = other.m_Map;
        m_CurrentPosition = other.m_CurrentPosition;
        m_MapStatus = other.m_MapStatus;
        other.m_MapStatus.reset();
        return *this;
    }

    ~IteratorBase() {
        if (!m_MapStatus.expired()) {
            m_Map->m_ActiveIterators.erase(m_CurrentPosition);
        }
    }

    IteratorBase & operator++() {
        skipToOccupied();
        return *this;
    }

    auto & operator*() const {
        isAlive();
        return *std::launder(reinterpret_cast<std::pair<const Key, Value>*>(&m_Map->m_Data.KVs[*m_CurrentPosition]));
    }

    auto operator->() const {
        isAlive();
        return std::launder(reinterpret_cast<std::pair<const Key, Value>*>(&m_Map->m_Data.KVs[*m_CurrentPosition]));
    }

    template<typename Iterator> requires concepts::isIterator<Iterator, Key, Value, Hash>
    bool operator==(const Iterator & other) const {
        if (*m_CurrentPosition != m_Map->m_Data.size()) isAlive();
        return *m_CurrentPosition == *other.m_CurrentPosition;
    }

    template<typename Iterator> requires concepts::isIterator<Iterator, Key, Value, Hash>
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
        do {
            ++(*m_CurrentPosition);
        } while (*m_CurrentPosition != m_Map->m_Data.size()
              && m_Map->m_Data.statuses[*m_CurrentPosition] != Status::OCCUPIED);
    }

    MapType * m_Map;
    ListIterator m_CurrentPosition;
    std::weak_ptr<bool> m_MapStatus;

    friend class flashmap;
    friend class IteratorBase<Value, flashmap>;
    friend class IteratorBase<const Value, const flashmap>;
};