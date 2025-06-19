#pragma once

template<typename Key, typename HashFunc>
concept Hashable = requires(Key key, HashFunc hasher)
{ { hasher(key) } -> std::convertible_to<std::size_t>; };

template<typename InputIt, typename Key, typename Value>
concept InputPairs = requires(InputIt it) {
    { (*it).first } -> std::convertible_to<Key>;
    { (*it).second } -> std::convertible_to<Value>;
};