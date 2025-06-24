#pragma once

// NESTED OBJECTS
namespace yulbax::container::flashmap::impl {

    enum class Status : uint8_t { FREE, OCCUPIED, DELETED };

    template<typename K, typename V>
    struct Vectors {
        explicit Vectors(std::size_t size) : KVs(size), statuses(size), hashes(size) {}

        std::vector<std::pair<K,V>> KVs;
        std::vector<Status> statuses;
        std::vector<std::size_t> hashes;

        std::tuple<std::pair<K,V>&, Status&, size_t&> operator[](const std::size_t index) {
            return {KVs[index], statuses[index], hashes[index]};
        }

        std::tuple<const std::pair<K,V>&, const Status&, const std::size_t&> operator[](const std::size_t index) const {
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



