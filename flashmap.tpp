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

    const std::size_t newhash = m_Hasher(key);
    std::size_t pos = getNextPosition(newhash);
    auto [kv, status, hash] = m_Data[pos];


    if (status == Status::OCCUPIED) {
        return false;
    }

    ++m_Count;
    kv = KeyValue(std::forward<K>(key), std::forward<V>(value));
    status = Status::OCCUPIED;
    hash = newhash;

    return true;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename K>
Value & flashmap<Key, Value, Hash>::operator[](K && key) {
    if (m_Count > m_MaxLoad) {
        rehash();
    }

    const std::size_t newhash = m_Hasher(key);
    std::size_t pos = getNextPosition(newhash);
    auto [kv, status, hash] = m_Data[pos];
    if (status != Status::OCCUPIED) {
        kv.key = std::forward<K>(key);
        status = Status::OCCUPIED;
        hash = newhash;

        ++m_Count;
    }

    return kv.value;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
Value & flashmap<Key, Value, Hash>::at(const Key & key) {
    std::size_t pos = findIndex(key);
    if (pos == m_Data.size()) {
        throw std::out_of_range("Key not found");
    }
    return m_Data.KVs[pos].value;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
const Value & flashmap<Key, Value, Hash>::at(const Key & key) const {
    std::size_t pos = findIndex(key);
    if (pos == m_Data.size()) {
        throw std::out_of_range("Key not found");
    }
    return m_Data.KVs[pos].value;
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
        m_Data.statuses[pos] = Status::DELETED;
        --m_Count;
        return true;
    }
    return false;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
void flashmap<Key, Value, Hash>::clear() {
    for (auto & status : m_Data.statuses) {
        status = Status::FREE;
    }
    m_ActiveIterators.clear();
    m_Count = 0;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::iterator
flashmap<Key, Value, Hash>::begin() {
    if (!m_Count) return end();

    std::size_t index = 0;
    while (m_Data.statuses[index] != Status::OCCUPIED) {
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
    while (m_Data.statuses[index] != Status::OCCUPIED) {
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
    Data oldData = std::move(m_Data);
    m_Data.resize(oldData.size() * 2);
    m_MaxLoad *= 2;

    for (auto & pos : m_ActiveIterators) {
        auto [oldKV, oldStatus, oldHash] = oldData[pos];
        std::size_t newPos = getNextPosition(oldHash);
        auto [newKV, newStatus, newHash] = m_Data[newPos];
        newKV = std::move(oldKV);
        newStatus = Status::OCCUPIED;
        newHash = oldHash;
        oldStatus = Status::DELETED;
        pos = newPos;
    }

    for (size_t i = 0; i < oldData.size(); ++i) {
        auto [oldKV, oldStatus, oldHash] = oldData[i];
        if (oldStatus != Status::OCCUPIED) continue;
        std::size_t newPos = getNextPosition(oldHash);
        auto [newKV, newStatus, newHash] = m_Data[newPos];
        newKV = std::move(oldKV);
        newStatus = Status::OCCUPIED;
        newHash = oldHash;
    }
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::nextCell(const std::size_t hash, const std::size_t shift) const {
    return (hash + shift + shift * shift) & (m_Data.size() - 1);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::findIndex(const Key & key) const {
    const std::size_t hash = m_Hasher(key);

    for (std::size_t shift = 0; ; ++shift) {
        std::size_t pos = nextCell(hash, shift);
        auto [kv, status, currentHash] = m_Data[pos];

        if (status == Status::FREE) break;

        if (status == Status::OCCUPIED && currentHash == hash) {
            return pos;
        }
    }

    return m_Data.size();
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::getNextPosition(const std::size_t hash) {
    std::size_t firstDeleted = m_Data.size();

    for (std::size_t shift = 0; ; ++shift) {
        std::size_t pos = nextCell(hash, shift);
        auto [kv, status, currentHash] = m_Data[pos];

        if (currentHash == hash && status == Status::OCCUPIED) return pos;

        if (status == Status::DELETED && firstDeleted == m_Data.size()) {
            firstDeleted = pos;
        }

        if (status == Status::FREE) {
            return firstDeleted != m_Data.size() ? firstDeleted : pos;
        }
    }
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
void flashmap<Key, Value, Hash>::loadFactor() {
    m_MaxLoad = static_cast<std::size_t>(static_cast<float>(m_Data.size()) * LOAD_FACTOR);
}