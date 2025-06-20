# FlashMap

A high-performance, template-based hash map implementation using open addressing with quadratic probing. This container provides O(1) average-case performance for insertions, lookups, and deletions while maintaining memory efficiency through flat storage. **The key feature of this implementation is stable iterators that remain valid even during rehashing operations.**

## Features

- **Open Addressing**: Uses quadratic probing for collision resolution
- **Template-based**: Generic implementation supporting any key-value types
- **Power-of-Two Sizing**: Automatic construction with power-of-two size
- **Automatic Rehashing**: Dynamically resizes when load factor exceeds threshold
- **STL-Compatible**: Provides iterators and familiar interface
- **Memory Efficient**: Flat array storage with minimal overhead
- **Stable Iterators**: Iterators remain valid during rehashing operations
- **C++20 Support**: Modern C++ features including concepts and structured bindings

## Requirements

- C++20 compatible compiler
- Standard library support for `<vector>` and `<list>`

## Usage

### Basic Operations

```cpp
#include "flashmap.hpp"

// Create a hash map with default size (1024)
yulbax::flashmap<int, std::string> map;

// Insert key-value pairs
map.insert(1, "hello");
map.insert(2, "world");

// Access elements (creates if not exists)
map[3] = "new value";

// Check if key exists
if (map.contains(1)) {
    std::cout << "Key 1 exists\n";
}

// Get value with bounds checking
try {
    std::string& value = map.at(1);
    std::cout << "Value: " << value << "\n";
} catch (const std::out_of_range& e) {
    std::cout << "Key not found\n";
}

// Remove elements
map.erase(2);

// Get container size
std::cout << "Size: " << map.size() << "\n";

// Clear all elements
map.clear();
```

### Custom Size and Hash Function

```cpp
// Custom size (will be scaled automatically to power of 2)
yulbax::flashmap<std::string, int> customSizeMap(512);

// Custom hash function
struct CustomHash {
    std::size_t operator()(const std::string& key) const {
        return std::hash<std::string>{}(key) * 31;
    }
};

yulbax::flashmap<std::string, int, CustomHash> customHashMap;
```

### Iteration with Stable Iterators

```cpp
yulbax::flashmap<int, std::string> map;
map[1] = "one";
map[2] = "two";
map[3] = "three";

// Range-based for loop with structured bindings
for (const auto& [key, value] : map) {
    std::cout << key << ": " << value << "\n";
}

// Iterator-based traversal
for (auto it = map.begin(); it != map.end(); ++it) {
    auto& [key, value] = *it;
    std::cout << key << ": " << value << "\n";
}

// Find specific element
auto it = map.find(2);
if (it != map.end()) {
    std::cout << "Found: " << it->first << " -> " << it->second << "\n";
}

// Stable iterators demo - iterator remains valid even during rehashing
auto iter = map.find(1);
// Insert many elements to trigger rehashing
for (int i = 100; i < 2000; ++i) {
    map[i] = "value" + std::to_string(i);
}
// Iterator is still valid!
std::cout << "Still valid: " << iter->first << " -> " << iter->second << "\n";
```

## How It Works

### Open Addressing with Quadratic Probing

FlashMap uses **open addressing** instead of chaining for collision resolution. When a collision occurs, it uses **quadratic probing** to find the next available slot:

```
nextPosition = (hash + i²) % tableSize
```

This approach provides:
- Better cache locality compared to chaining
- Reduced memory overhead (no linked lists for collision handling)
- Predictable memory access patterns

### Storage Strategy

Each element in the hash table has three states:
- **FREE**: Never used
- **OCCUPIED**: Contains valid key-value pair
- **DELETED**: Previously occupied but erased (tombstone)

This tombstone approach allows for efficient deletion without disrupting probe sequences.

### Stable Iterator Management

The key innovation of this implementation is the stable iterator system:
- All active iterators are tracked in `m_ActiveIterators` list
- During rehashing, iterators are automatically updated to point to new positions
- Iterator destructors automatically remove themselves from the active list
- Both mutable and const iterators are fully supported
- Copy/assignment operators properly manage iterator lifecycle

### Automatic Rehashing

The container automatically rehashes when the load factor exceeds 87.5%. During rehashing:
1. Table size doubles (maintaining power-of-2 constraint)
2. All elements are rehashed into new positions
3. **All active iterators are automatically updated to new positions**
4. Deleted elements are cleaned up

### Performance Characteristics

- **Average Case**: O(1) for insert, lookup, delete
- **Space Complexity**: O(n) with low overhead
- **Load Factor**: Maintained below 87.5% for optimal performance
- **Iterator Stability**: O(k) overhead during rehashing, where k is the number of active iterators

## Template Parameters

| Parameter | Description        | Default          | Constraints                     |
|-----------|--------------------|------------------|---------------------------------|
| `Key`     | Key type           | -                | Must be equality comparable     |
| `Value`   | Value type         | -                | Must be copy/move constructible |
| `Hash`    | Hash function type | `std::hash<Key>` | Must satisfy `Hashable` concept |

## API Reference

### Constructors
```cpp
explicit flashmap(std::size_t size = DEFAULT_SIZE);
flashmap(const FlashMap& other);
flashmap(Iter begin, Iter end);
flashmap& operator=(const FlashMap& other);
```

### Modification
```cpp
template<typename K, typename V>
bool insert(K&& key, V&& value);          // Insert element

template<typename K>
Value& operator[](K&& key);               // Access with creation

bool erase(const Key& key);               // Remove element
void clear();                             // Clear all elements
```

### Access
```cpp
Value& at(const Key& key);                // Access with bounds checking
const Value& at(const Key& key) const;    // Const access with bounds checking
bool contains(const Key& key) const;      // Check existence
std::size_t size() const;                 // Container size
```

### Iterators
```cpp
iterator begin();                          // Begin iterator
const_iterator begin() const;              // Const begin iterator
iterator end();                            // End iterator  
const_iterator end() const;                // Const end iterator
iterator find(const Key& key);             // Find element
const_iterator find(const Key& key) const; // Const find
```

## C++20 Concepts

Uses the `Hashable` concept for compile-time type checking:
```cpp
template<typename Key, typename HashFunc>
concept Hashable = requires(Key key, HashFunc hasher)
{ { hasher(key) } -> std::convertible_to<std::size_t>; };
```

## Structured Bindings Support

Full support for structured bindings through `std::tuple_element` specialization:
```cpp
for (auto& [key, value] : map) {
    // key is const Key&, value is Value&
}
```

## Constants

```cpp
static constexpr std::size_t DEFAULT_SIZE = 1024;  // Default size
static constexpr float LOAD_FACTOR = 0.875;        // Load factor threshold
```

## Thread Safety

This container is **not thread-safe**. External synchronization is required for concurrent access.

## Implementation Notes

- Uses `std::vector` for underlying storage
- Uses `std::list` for tracking active iterators
- Bitwise AND operation for fast modulo (size automatically scales to power-of-2)
- Perfect forwarding for efficient key-value insertion
- Automatic iterator lifecycle management
- Support for structured bindings via `std::tuple_element` specialization

## Performance Comparison
Benchmark results comparing FlashMap with std::unordered_map (100,000 iterations):
| Container               | Avg Time (seconds) |
|-------------------------|--------------------|
| `FlashMap`              | **0.0122**         |
| `std::unordered_map`    | 0.0182             |
| `ratio (flash/std)`     | 0.67               |

*Note: Stable iterator feature adds minimal overhead during normal operations and only affects performance during rehashing operations.*