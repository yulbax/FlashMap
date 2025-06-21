#pragma once

namespace yulbax::concepts {
    template<typename Key, typename HashFunc>
    concept hashable = requires(Key key, HashFunc hasher)
    { { hasher(key) } -> std::convertible_to<std::size_t>; };

    template<typename InputIt, typename Key, typename Value>
    concept inititerator = requires(InputIt it) {
        { (*it).first } -> std::convertible_to<Key>;
        { (*it).second } -> std::convertible_to<Value>;
    };
}