#pragma once

// NESTED OBJECTS

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
enum class flashmap<Key, Value, Hash>::Status { FREE, OCCUPIED, DELETED };

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
class flashmap<Key, Value, Hash>::KeyValue {
public:
    KeyValue() = default;

    template<typename K, typename V>
    KeyValue(K && k, V && v)
        : key(std::forward<K>(k)), value(std::forward<V>(v)) {
    }

    Key key;
    Value value;

    explicit operator std::pair<Key, Value>() const & {
        return {key, value};
    }

    explicit operator std::pair<Key, Value>() && {
        return {std::move(key), std::move(value)};
    }

    template<std::size_t I>
    auto & get() & {
        if constexpr (I == 0) {
            return static_cast<const Key &>(key);
        } else if constexpr (I == 1) {
            return value;
        }
    }

    template<std::size_t I>
    const auto & get() const & {
        if constexpr (I == 0) {
            return key;
        } else if constexpr (I == 1) {
            return value;
        }
    }

    template<std::size_t I>
    auto && get() && {
        if constexpr (I == 0) {
            return static_cast<const Key &&>(std::move(key));
        } else if constexpr (I == 1) {
            return std::move(value);
        }
    }

    template<std::size_t I>
    const auto && get() const && {
        if constexpr (I == 0) {
            return std::move(key);
        } else if constexpr (I == 1) {
            return std::move(value);
        }
    }
};

template<typename Key, typename Value, typename Hash> requires yulbax::concepts::hashable<Key, Hash>
class flashmap<Key, Value, Hash>::Element {
public:
    Element() : kv(), status(Status::FREE) {}

    template<typename K, typename V>
    Element(K && key, V && value)
        : kv(std::forward<K>(key), std::forward<V>(value)), status(Status::OCCUPIED) {
    }

    KeyValue kv;
    Status status;
};


