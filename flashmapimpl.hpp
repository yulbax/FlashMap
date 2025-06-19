#pragma once

// NESTED OBJECTS
namespace FlashMapImpl {
    enum class Status { FREE, OCCUPIED, DELETED };

    template<typename Key, typename Value>
    class KeyValue {
    public:
        KeyValue() = default;

        template <typename K, typename V>
        KeyValue(K && k, V && v)
            : key(std::forward<K>(k)), value(std::forward<V>(v)) {}

        Key key;
        Value value;

        explicit operator std::pair<Key, Value>() const& {
            return {key, value};
        }

        explicit operator std::pair<Key, Value>() && {
            return {std::move(key), std::move(value)};
        }

        template<std::size_t I>
        auto & get() & {
            if constexpr (I == 0) {
                return static_cast<const Key&>(key);
            } else if constexpr (I == 1) {
                return value;
            }
        }

        template<std::size_t I>
        const auto & get() const& {
            if constexpr (I == 0) {
                return key;
            } else if constexpr (I == 1) {
                return value;
            }
        }

        template<std::size_t I>
        auto && get() && {
            if constexpr (I == 0) {
                return static_cast<const Key&&>(std::move(key));
            } else if constexpr (I == 1) {
                return std::move(value);
            }
        }

        template<std::size_t I>
        const auto && get() const&& {
            if constexpr (I == 0) {
                return std::move(key);
            } else if constexpr (I == 1) {
                return std::move(value);
            }
        }
    };

    template<typename Key, typename Value>
    class Element {
    public:
        Element() : kv(), status(Status::FREE) {}

        template <typename K, typename V>
        Element(K && key, V && value)
            : kv(std::forward<K>(key), std::forward<V>(value)), status(Status::OCCUPIED) {
        }

        KeyValue<Key, Value> kv;
        Status status;
    };
}

namespace std {
    template<typename Key, typename Value>
    struct tuple_size<FlashMapImpl::KeyValue<Key, Value>> { // NOLINT(*-dcl58-cpp)
        static constexpr std::size_t value = 2;
    };

    template<std::size_t I, typename Key, typename Value>
    struct tuple_element<I, FlashMapImpl::KeyValue<Key, Value>> { // NOLINT(*-dcl58-cpp)
        using type = std::conditional_t<I == 0, const Key, Value>;
    };

    template<std::size_t I, typename Key, typename Value>
    struct tuple_element<I, const FlashMapImpl::KeyValue<Key, Value>> { // NOLINT(*-dcl58-cpp)
        using type = std::conditional_t<I == 0, const Key, const Value>;
    };
}
