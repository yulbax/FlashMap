#pragma once
#include "flashmap.hpp"

// PUBLIC METHODS
template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
FlashMap<Key, Value, Hash>::FlashMap(std::size_t size)
    : m_Data(std::bit_ceil(size)), m_Hasher(), m_Count(0), m_MaxLoad(), m_ActiveIterators() {
    loadFactor();
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
template<typename InputIt> requires InputPairs<InputIt, Key, Value>
FlashMap<Key, Value, Hash>::FlashMap(InputIt first, InputIt last) : FlashMap() {
    for (; first != last; ++first)
        insert(first->first, first->second);
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
FlashMap<Key, Value, Hash>::FlashMap(const FlashMap & other) : m_Data(other.m_Data), m_Hasher(other.m_Hasher),
                                                                        m_Count(other.m_Count), m_MaxLoad(other.m_MaxLoad),
                                                                        m_ActiveIterators() {}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
FlashMap<Key, Value, Hash> & FlashMap<Key, Value, Hash>::operator=(const FlashMap & other) {
    if (this == &other) return *this;
    m_Data = other.m_Data;
    m_Hasher = other.m_Hasher;
    m_Count = other.m_Count;
    m_MaxLoad = other.m_MaxLoad;
    m_ActiveIterators.clear();
    return *this;
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
template<typename K, typename V>
bool FlashMap<Key, Value, Hash>::insert(K && key, V && value) {
    if (m_Count > m_MaxLoad) {
        rehash();
    }

    auto it = getNextPosition(key);

    if (it->status == Status::OCCUPIED) {
        return false;
    }

    ++m_Count;
    *it = Element(std::forward<K>(key), std::forward<V>(value));

    return true;
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
template<typename K>
Value & FlashMap<Key, Value, Hash>::operator[](K && key) {
    if (m_Count > m_MaxLoad) {
        rehash();
    }

    auto it = getNextPosition(key);
    if (it->status != Status::OCCUPIED) {
        it->kv.key = std::forward<K>(key);
        it->status = Status::OCCUPIED;
        ++m_Count;
    }

    return it->kv.value;
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
Value & FlashMap<Key, Value, Hash>::at(const Key & key) {
    auto it = findIndex(key);
    if (it == m_Data.end()) {
        throw std::out_of_range("Key not found");
    }
    return it->kv.value;
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
const Value & FlashMap<Key, Value, Hash>::at(const Key & key) const {
    auto it = findIndex(key);
    if (it == m_Data.end()) {
        throw std::out_of_range("Key not found");
    }
    return it->kv.value;
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
bool FlashMap<Key, Value, Hash>::contains(const Key & key) const {
    return findIndex(key) != m_Data.end();
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
std::size_t FlashMap<Key, Value, Hash>::size() const {
    return m_Count;
}


template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
bool FlashMap<Key, Value, Hash>::erase(const Key & key) {
    auto it = findIndex(key);
    if (it != m_Data.end()) {
        it->status = Status::DELETED;
        --m_Count;
        return true;
    }
    return false;
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
void FlashMap<Key, Value, Hash>::clear() {
    for (auto & cell : m_Data) {
        cell.status = Status::FREE;
    }
    m_ActiveIterators.clear();
    m_Count = 0;
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
typename FlashMap<Key, Value, Hash>::iterator
FlashMap<Key, Value, Hash>::begin() {
    if (!m_Count) return end();

    std::size_t index = 0;
    while (m_Data[index].status != Status::OCCUPIED) {
        ++index;
    }

    auto it = m_Data.begin() + index;
    m_ActiveIterators.emplace_front(it);
    auto listIt = m_ActiveIterators.begin();

    return iterator(listIt, *this);
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
typename FlashMap<Key, Value, Hash>::const_iterator
FlashMap<Key, Value, Hash>::begin() const {
    if (!m_Count) return end();

    std::size_t index = 0;
    while (m_Data[index].status != Status::OCCUPIED) {
        ++index;
    }

    auto it = m_Data.begin() + index;
    m_ActiveIterators.emplace_front(it);
    auto listIt = m_ActiveIterators.begin();

    return const_iterator(listIt, *this);
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
typename FlashMap<Key, Value, Hash>::iterator
FlashMap<Key, Value, Hash>::end() {
    m_ActiveIterators.emplace_front(m_Data.end());
    auto listIt = m_ActiveIterators.begin();

    return iterator(listIt, *this);
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
typename FlashMap<Key, Value, Hash>::const_iterator
FlashMap<Key, Value, Hash>::end() const {
    m_ActiveIterators.emplace_front(m_Data.end());
    auto listIt = m_ActiveIterators.begin();
    return const_iterator(listIt, *this);
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
typename FlashMap<Key, Value, Hash>::iterator
FlashMap<Key, Value, Hash>::find(const Key & key) {
    auto it = findIndex(key);
    if (it == m_Data.end()) {
        return end();
    }
    m_ActiveIterators.emplace_front(it);
    auto listIt = m_ActiveIterators.begin();
    return iterator(listIt, *this);
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
typename FlashMap<Key, Value, Hash>::const_iterator
FlashMap<Key, Value, Hash>::find(const Key & key) const {
    auto it = findIndex(key);
    if (it == m_Data.end()) {
        return end();
    }
    m_ActiveIterators.emplace_front(it);
    auto listIt = m_ActiveIterators.begin();

    return const_iterator(listIt, *this);
}


// PRIVATE METHODS
template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
std::size_t FlashMap<Key, Value, Hash>::hash(const Key & key) const {
    return m_Hasher(key);
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
void FlashMap<Key, Value, Hash>::rehash() {
    std::vector<Element> oldData = std::move(m_Data);
    m_Data.resize(oldData.size() * 2);
    loadFactor();

    for (auto & it : m_ActiveIterators) {
        auto newIt = getNextPosition(it->kv.key);
        newIt->kv = std::move(it->kv);
        newIt->status = Status::OCCUPIED;
        it->status = Status::DELETED;
        it = newIt;
    }

    for (auto & element : oldData) {
        if (element.status == Status::OCCUPIED) {
            auto it = getNextPosition(element.kv.key);
            it->kv = std::move(element.kv);
            it->status = Status::OCCUPIED;
        }
    }
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
std::size_t FlashMap<Key, Value, Hash>::nextCell(const std::size_t index, const std::size_t shift) const {
    return (index + shift * shift) & (m_Data.size() - 1);
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
auto FlashMap<Key, Value, Hash>::findIndex(const Key & key) {
    const std::size_t index = hash(key);
    auto current = m_Data.begin();

    for (std::size_t shift = 0; ; ++shift) {
        std::size_t pos = nextCell(index, shift);
        current = m_Data.begin() + pos;

        if (current->status == Status::FREE) break;

        if (current->status == Status::OCCUPIED && current->kv.key == key) {
            return current;
        }
    }

    return m_Data.end();
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
auto FlashMap<Key, Value, Hash>::findIndex(const Key & key) const {
    const std::size_t index = hash(key);
    auto current = m_Data.begin();

    for (std::size_t shift = 0; ; ++shift) {
        current = m_Data.begin() + nextCell(index, shift);

        if (current->status == Status::FREE) break;

        if (current->status == Status::OCCUPIED && current->kv.key == key) {
            return current;
        }
    }

    return m_Data.end();
}


template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
auto FlashMap<Key, Value, Hash>::getNextPosition(const Key & key) {
    const std::size_t index = hash(key);
    auto firstDeleted = m_Data.end();

    for (std::size_t shift = 0; ; ++shift) {
        std::size_t pos = nextCell(index, shift);
        auto current = m_Data.begin() + pos;

        if (current->kv.key == key && current->status == Status::OCCUPIED) return current;

        if (current->status == Status::DELETED && firstDeleted == m_Data.end()) {
            firstDeleted = current;
        }

        if (current->status == Status::FREE) {
            return firstDeleted != m_Data.end() ? firstDeleted : current;
        }
    }
}

template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
void FlashMap<Key, Value, Hash>::killIterator(ListIterator iter) const {
    m_ActiveIterators.erase(iter);
}


template<typename Key, typename Value, typename Hash> requires Hashable<Key, Hash>
void FlashMap<Key, Value, Hash>::loadFactor() const {
    m_MaxLoad = static_cast<std::size_t>(static_cast<float>(m_Data.size()) * LOAD_FACTOR);
}