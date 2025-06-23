#pragma once

// PUBLIC METHODS
template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
flashmap<Key, Value, Hash>::flashmap(std::size_t size)
    : m_Data(std::bit_ceil(size)), m_Hasher(), m_Count(0), m_MaxLoad(), m_ActiveIterators() {
    loadFactor();
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename InputIt> requires yulbax::concepts::inititerator<InputIt, Key, Value>
flashmap<Key, Value, Hash>::flashmap(InputIt first, InputIt last) : flashmap() {
    for (; first != last; ++first)
        insert(first->first, first->second);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
flashmap<Key, Value, Hash>::flashmap(const flashmap & other) : m_Data(other.m_Data), m_Hasher(other.m_Hasher),
                                                               m_Count(other.m_Count), m_MaxLoad(other.m_MaxLoad),
                                                               m_ActiveIterators() {}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
flashmap<Key, Value, Hash> & flashmap<Key, Value, Hash>::operator=(const flashmap & other) {
    if (this == &other) return *this;
    m_Data = other.m_Data;
    m_Hasher = other.m_Hasher;
    m_Count = other.m_Count;
    m_MaxLoad = other.m_MaxLoad;
    m_ActiveIterators.clear();
    return *this;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename K, typename V>
bool flashmap<Key, Value, Hash>::insert(K && key, V && value) {
    if (m_Count > m_MaxLoad) {
        rehash();
    }

    std::size_t pos = getNextPosition(key);
    auto & current = m_Data[pos];

    if (current.status == Status::OCCUPIED) {
        return false;
    }

    ++m_Count;
    current = Element(std::forward<K>(key), std::forward<V>(value));

    return true;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename K>
Value & flashmap<Key, Value, Hash>::operator[](K && key) {
    if (m_Count > m_MaxLoad) {
        rehash();
    }

    std::size_t pos = getNextPosition(key);
    auto & current = m_Data[pos];
    if (current.status != Status::OCCUPIED) {
        current.kv.key = std::forward<K>(key);
        current.status = Status::OCCUPIED;
        ++m_Count;
    }

    return current.kv.value;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
Value & flashmap<Key, Value, Hash>::at(const Key & key) {
    std::size_t pos = findIndex(key);
    if (pos == m_Data.size()) {
        throw std::out_of_range("Key not found");
    }
    return m_Data[pos].kv.value;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
const Value & flashmap<Key, Value, Hash>::at(const Key & key) const {
    std::size_t pos = findIndex(key);
    if (pos == m_Data.size()) {
        throw std::out_of_range("Key not found");
    }
    return m_Data[pos].kv.value;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
bool flashmap<Key, Value, Hash>::contains(const Key & key) const {
    return findIndex(key) != m_Data.size();
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::size() const {
    return m_Count;
}


template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
bool flashmap<Key, Value, Hash>::erase(const Key & key) {
    std::size_t pos = findIndex(key);
    if (pos != m_Data.size()) {
        m_Data[pos].status = Status::DELETED;
        --m_Count;
        return true;
    }
    return false;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
void flashmap<Key, Value, Hash>::clear() {
    for (auto & cell : m_Data) {
        cell.status = Status::FREE;
    }
    m_ActiveIterators.clear();
    m_Count = 0;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::iterator
flashmap<Key, Value, Hash>::begin() {
    if (!m_Count) return end();

    std::size_t index = 0;
    while (m_Data[index].status != Status::OCCUPIED) {
        ++index;
    }

    m_ActiveIterators.emplace_front(index);
    auto listIt = m_ActiveIterators.begin();

    return iterator(listIt, this);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::const_iterator
flashmap<Key, Value, Hash>::begin() const {
    if (!m_Count) return end();

    std::size_t index = 0;
    while (m_Data[index].status != Status::OCCUPIED) {
        ++index;
    }

    m_ActiveIterators.emplace_front(index);
    auto listIt = m_ActiveIterators.begin();
    return const_iterator(listIt, this);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::iterator
flashmap<Key, Value, Hash>::end() {
    m_ActiveIterators.emplace_front(m_Data.size());
    auto listIt = m_ActiveIterators.begin();
    return iterator(listIt, this);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::const_iterator
flashmap<Key, Value, Hash>::end() const {
    m_ActiveIterators.emplace_front(m_Data.size());
    auto listIt = m_ActiveIterators.begin();
    return const_iterator(listIt, this);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::iterator
flashmap<Key, Value, Hash>::find(const Key & key) {
    const std::size_t pos = findIndex(key);
    if (pos == m_Data.size()) {
        return end();
    }
    m_ActiveIterators.emplace_front(pos);
    auto listIt = m_ActiveIterators.begin();
    return iterator(listIt, this);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::const_iterator
flashmap<Key, Value, Hash>::find(const Key & key) const {
    const std::size_t pos = findIndex(key);
    if (pos == m_Data.size()) {
        return end();
    }
    m_ActiveIterators.emplace_front(pos);
    auto listIt = m_ActiveIterators.begin();
    return const_iterator(listIt, this);
}


// PRIVATE METHODS
template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
void flashmap<Key, Value, Hash>::rehash() {
    std::vector<Element> oldData = std::move(m_Data);
    m_Data.resize(oldData.size() * 2);
    m_MaxLoad *= 2;

    for (auto & pos : m_ActiveIterators) {
        auto & oldCell = oldData[pos];
        std::size_t newPos = getNextPosition(oldCell.kv.key);
        auto & newCell = m_Data[newPos];
        newCell.kv = std::move(oldCell.kv);
        newCell.status = Status::OCCUPIED;
        oldCell.status = Status::DELETED;
        pos = newPos;
    }

    for (auto & element : oldData) {
        if (element.status == Status::OCCUPIED) {
            std::size_t newPos = getNextPosition(element.kv.key);
            auto & newCell = m_Data[newPos];
            newCell.kv = std::move(element.kv);
            newCell.status = Status::OCCUPIED;
        }
    }
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::nextCell(const std::size_t index, const std::size_t shift) const {
    return (index + shift + shift * shift) & (m_Data.size() - 1);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::findIndex(const Key & key) {
    const std::size_t index = m_Hasher(key);

    for (std::size_t shift = 0; ; ++shift) {
        std::size_t pos = nextCell(index, shift);
        auto & current = m_Data[pos];

        if (current.status == Status::FREE) break;

        if (current.status == Status::OCCUPIED && current.kv.key == key) {
            return pos;
        }
    }

    return m_Data.size();
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::findIndex(const Key & key) const {
    const std::size_t index = m_Hasher(key);

    for (std::size_t shift = 0; ; ++shift) {
        std::size_t pos = nextCell(index, shift);
        auto & current = m_Data[pos];

        if (current.status == Status::FREE) break;

        if (current.status == Status::OCCUPIED && current.kv.key == key) {
            return pos;
        }
    }

    return m_Data.size();
}


template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::getNextPosition(const Key & key) {
    const std::size_t index = m_Hasher(key);
    auto firstDeleted = m_Data.size();

    for (std::size_t shift = 0; ; ++shift) {
        std::size_t pos = nextCell(index, shift);
        auto & current = m_Data[pos];

        if (current.kv.key == key && current.status == Status::OCCUPIED) return pos;

        if (current.status == Status::DELETED && firstDeleted == m_Data.size()) {
            firstDeleted = pos;
        }

        if (current.status == Status::FREE) {
            return firstDeleted != m_Data.size() ? firstDeleted : pos;
        }
    }
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
void flashmap<Key, Value, Hash>::loadFactor() {
    m_MaxLoad = static_cast<std::size_t>(static_cast<float>(m_Data.size()) * LOAD_FACTOR);
}