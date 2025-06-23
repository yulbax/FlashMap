#pragma once

// NESTED OBJECTS
namespace yulbax::container::flashmap::impl {
    template<typename Key, typename Value>
    class KeyValue {
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

    enum class Status : uint8_t { FREE, OCCUPIED, DELETED };

    template<typename K, typename V>
    struct Vectors {
        explicit Vectors(std::size_t size) : KVs(size), statuses(size), hashes(size) {}

        std::vector<KeyValue<K,V>> KVs;
        std::vector<Status> statuses;
        std::vector<std::size_t> hashes;

        std::tuple<KeyValue<K,V>&, Status&, size_t&> operator[](const std::size_t index) {
            return {KVs[index], statuses[index], hashes[index]};
        }

        std::tuple<const KeyValue<K,V>&, const Status&, const std::size_t&> operator[](const std::size_t index) const {
            return {KVs[index], statuses[index], hashes[index]};
        }

        [[nodiscard]] std::size_t size() const {
            return KVs.size();
        }

        void resize(const std::size_t size) {
            KVs.resize(size);
            statuses.resize(size);
            hashes.resize(size);
        }
    };
}



