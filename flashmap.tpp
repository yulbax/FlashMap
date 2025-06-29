#pragma once

// PUBLIC METHODS
template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
flashmap<Key, Value, Hash>::flashmap(const std::size_t size) : m_Data(std::bit_ceil(size)), m_Hasher(),
                                                               m_Count(0), m_MaxLoad(loadFactor()),
                                                               endIt(this, m_Data.size()),
                                                               cendIt(this, m_Data.size()) {}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename InputIt> requires yulbax::concepts::inititerator<InputIt, Key, Value>
flashmap<Key, Value, Hash>::flashmap(InputIt first, InputIt last) : flashmap() {
    for (; first != last; ++first)
        insert(first->first, first->second);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
flashmap<Key, Value, Hash>::flashmap(const flashmap & other) : m_Data(other.m_Data), m_Hasher(other.m_Hasher),
                                                               m_Count(other.m_Count), m_MaxLoad(other.m_MaxLoad),
                                                               endIt(this, m_Data.size()),
                                                               cendIt(this, m_Data.size()) {}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
flashmap<Key, Value, Hash> & flashmap<Key, Value, Hash>::operator=(const flashmap & other) {
    if (this == &other) return *this;
    invalidateIterators();
    m_Data = other.m_Data;
    m_Hasher = other.m_Hasher;
    m_Count = other.m_Count;
    m_MaxLoad = other.m_MaxLoad;
    m_ActiveIterators.erase(std::next(m_ActiveIterators.begin(), 2), m_ActiveIterators.end());
    endIt.m_Index = m_Data.size();
    cendIt.m_Index = m_Data.size();
    return *this;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
flashmap<Key, Value, Hash>::flashmap(flashmap && other) noexcept
    : m_Data(std::move(other.m_Data)),
      m_Hasher(std::move(other.m_Hasher)),
      m_Count(other.m_Count),
      m_MaxLoad(other.m_MaxLoad),
      m_ActiveIterators(std::move(other.m_ActiveIterators)),
      endIt(std::move(other.endIt)),
      cendIt(std::move(other.cendIt)) {
    updateIterators();
    other.m_Data.resize(4);
    other.m_Count = 0;
    other.m_MaxLoad = 3;
    other.endIt.m_Index = 4;
    other.cendIt.m_Index = 4;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
flashmap<Key, Value, Hash> & flashmap<Key, Value, Hash>::operator=(flashmap && other) noexcept {
    if (this == &other) return *this;
    invalidateIterators();
    m_Data = std::move(other.m_Data);
    m_Hasher = std::move(other.m_Hasher);
    m_Count = other.m_Count;
    m_MaxLoad = other.m_MaxLoad;
    m_ActiveIterators = std::move(other.m_ActiveIterators);
    endIt = std::move(other.endIt);
    cendIt = std::move(other.cendIt);
    updateIterators();
    other.m_Data.resize(4);
    other.m_Count = 0;
    other.m_MaxLoad = 3;
    other.endIt.m_Index = 4;
    other.cendIt.m_Index = 4;
    return *this;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
flashmap<Key, Value, Hash>::~flashmap() {
    invalidateIterators();
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename K, typename V>
bool flashmap<Key, Value, Hash>::insert(K && key, V && value) {
    if (m_Count > m_MaxLoad) rehash();

    const HashType newhash = m_Hasher(key);
    std::size_t pos = getNextPosition(key, newhash);
    auto [kv, status, hash] = m_Data[pos];

    if (status == Status::OCCUPIED) return false;

    ++m_Count;
    kv.first = std::forward<K>(key);
    kv.second = std::forward<V>(value);
    status = Status::OCCUPIED;
    hash = newhash;

    return true;
}

template<typename Key, typename Value, typename Hash> requires concepts::hashable<Key, Hash>
template<typename K, typename V>
std::pair<typename flashmap<Key, Value, Hash>::iterator, bool> flashmap<Key, Value, Hash>::emplace(K && key, V && value)  {
    if (m_Count > m_MaxLoad) rehash();

    const HashType newhash = m_Hasher(key);
    std::size_t pos = getNextPosition(key, newhash);
    auto [kv, status, hash] = m_Data[pos];

    if (status == Status::OCCUPIED) return {iterator(this, pos), false};

    ++m_Count;
    kv.first = std::forward<K>(key);
    kv.second = std::forward<V>(value);
    status = Status::OCCUPIED;
    hash = newhash;

    return {iterator(this, pos), true};
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename K>
Value & flashmap<Key, Value, Hash>::operator[](K && key) {
    if (m_Count > m_MaxLoad) rehash();

    const HashType newhash = m_Hasher(key);
    std::size_t pos = getNextPosition(key, newhash);
    auto [kv, status, hash] = m_Data[pos];
    if (status != Status::OCCUPIED) {
        kv.first = std::forward<K>(key);
        status = Status::OCCUPIED;
        hash = newhash;
        ++m_Count;
    }

    return kv.second;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
Value & flashmap<Key, Value, Hash>::at(const Key & key) {
    std::size_t pos = findIndex(key);
    if (pos == m_Data.size()) throw std::out_of_range("Key not found");
    return m_Data.KVs[pos].second;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
const Value & flashmap<Key, Value, Hash>::at(const Key & key) const {
    std::size_t pos = findIndex(key);
    if (pos == m_Data.size()) throw std::out_of_range("Key not found");
    return m_Data.KVs[pos].second;
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

template<typename Key, typename Value, typename Hash> requires concepts::hashable<Key, Hash>
bool flashmap<Key, Value, Hash>::erase(iterator & it)  {
    if (it.m_Map != this) return false;

    std::size_t pos = it.m_Index;
    if (pos >= m_Data.size() || m_Data.statuses[pos] == Status::DELETED) return false;
    m_Data.statuses[pos] = Status::DELETED;
    --m_Count;
    return true;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
void flashmap<Key, Value, Hash>::clear() {
    std::ranges::fill(m_Data.statuses, Status::FREE);
    invalidateIterators();
    m_ActiveIterators.erase(std::next(m_ActiveIterators.begin(), 2), m_ActiveIterators.end());
    endIt.m_Map = this; cendIt.m_Map = this;
    m_Count = 0;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::iterator
flashmap<Key, Value, Hash>::begin() {
    if (!m_Count) return end();
    auto pos = std::ranges::find(m_Data.statuses, Status::OCCUPIED);
    std::size_t index = pos - m_Data.statuses.begin();
    return iterator(this, index);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::const_iterator
flashmap<Key, Value, Hash>::begin() const {
    if (!m_Count) return end();
    auto pos = std::ranges::find(m_Data.statuses, Status::OCCUPIED);
    std::size_t index = pos - m_Data.statuses.begin();
    return const_iterator(this, index);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::iterator &
flashmap<Key, Value, Hash>::end() {
    return endIt;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
const typename flashmap<Key, Value, Hash>::const_iterator &
flashmap<Key, Value, Hash>::end() const {
    return cendIt;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::iterator
flashmap<Key, Value, Hash>::find(const Key & key) {
    const std::size_t pos = findIndex(key);
    if (pos == m_Data.size()) return end();
    return iterator(this, pos);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
typename flashmap<Key, Value, Hash>::const_iterator
flashmap<Key, Value, Hash>::find(const Key & key) const {
    const std::size_t pos = findIndex(key);
    if (pos == m_Data.size()) return end();
    return const_iterator(this, pos);
}

// PRIVATE METHODS
template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
void flashmap<Key, Value, Hash>::rehash() {
    Data oldData = std::move(m_Data);
    std::size_t newSize = oldData.size() * 2;
    m_Data.resize(newSize);
    m_MaxLoad *= 2;
    auto iterStart = std::next(m_ActiveIterators.begin(), 2);
    endIt.m_Index = newSize; cendIt.m_Index = newSize;

    if (m_ActiveIterators.size() > 2) {
        flashmap<std::size_t, std::size_t> updatedPositions(m_ActiveIterators.size());

        for (auto & ptr : std::ranges::subrange(iterStart, m_ActiveIterators.end())) {
            std::visit([&](auto * it) {
                if (updatedPositions.contains(it->m_Index)) {
                    it->m_Index = updatedPositions.at(it->m_Index);
                    return;
                }

                auto [oldKV, oldStatus, oldHash] = oldData[it->m_Index];
                if (oldStatus != Status::OCCUPIED) return;

                std::size_t newPos = getNextPosition(oldKV.first, oldHash);
                auto [newKV, newStatus, newHash] = m_Data[newPos];

                newKV = std::move(oldKV);
                newStatus = Status::OCCUPIED;
                newHash = oldHash;
                oldStatus = Status::DELETED;

                updatedPositions[it->m_Index] = newPos;
                it->m_Index = newPos;
            }, ptr);
        }
    }

    for (size_t i = 0; i < oldData.size(); ++i) {
        auto [oldKV, oldStatus, oldHash] = oldData[i];
        if (oldStatus != Status::OCCUPIED) continue;
        std::size_t newPos = getNextPosition(oldKV.first, oldHash);
        auto [newKV, newStatus, newHash] = m_Data[newPos];
        newKV = std::move(oldKV);
        newStatus = Status::OCCUPIED;
        newHash = oldHash;
    }
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::nextCell(const HashType hash, const std::size_t shift) const {
    return (hash + shift) & (m_Data.size() - 1);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::findIndex(const Key & key) const {
    const HashType hash = m_Hasher(key);

    for (std::size_t shift = 0; ; ++shift) {
        std::size_t pos = nextCell(hash, shift);
        auto [kv, status, currentHash] = m_Data[pos];

        if (status == Status::FREE) break;

        if (status == Status::OCCUPIED && currentHash == hash) {
#ifdef CHECK_KEY_EQUALITY
            if (key == kv.first)
#endif
            return pos;
        }
    }

    return m_Data.size();
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::getNextPosition(const Key & key, const HashType hash) {
    std::size_t firstDeleted = m_Data.size();

    for (std::size_t shift = 0; ; ++shift) {
        std::size_t pos = nextCell(hash, shift);
        auto [kv, status, currentHash] = m_Data[pos];

        if (currentHash == hash && status == Status::OCCUPIED) {
#ifdef CHECK_KEY_EQUALITY
            if (key == kv.first)
#endif
            return pos;
        }

        if (status == Status::DELETED && firstDeleted == m_Data.size()) {
            firstDeleted = pos;
        }

        if (status == Status::FREE) {
            return firstDeleted != m_Data.size() ? firstDeleted : pos;
        }
    }
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
std::size_t flashmap<Key, Value, Hash>::loadFactor() const {
    return m_Data.size() * LOAD_FACTOR;
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename T>
void flashmap<Key, Value, Hash>::registerIterator(T * it) const {
    it->m_IteratorNode = m_ActiveIterators.insert(m_ActiveIterators.end(), it);
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
template<typename T>
void flashmap<Key, Value, Hash>::unregisterIterator(T * it) const {
    if (it->m_IteratorNode != m_ActiveIterators.end()) {
        m_ActiveIterators.erase(it->m_IteratorNode);
        it->m_IteratorNode = m_ActiveIterators.end();
        it->m_Map = nullptr;
    }
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
void flashmap<Key, Value, Hash>::invalidateIterators(flashmap * map) {
    for (auto & ptr : m_ActiveIterators) {
        std::visit([&](auto * it) {
            it->m_Map = map;
        }, ptr);
    }
}

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
void flashmap<Key, Value, Hash>::updateIterators() {
    invalidateIterators(this);
}
