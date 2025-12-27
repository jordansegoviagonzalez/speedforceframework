#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept> // For std::runtime_error
#include <type_traits> // For std::is_same

namespace speedforce {

// Enum for tensor data types
enum class DataType {
    FLOAT32,
    FLOAT16, // Note: C++ does not have a native float16 type, typically requires a library or custom type
    INT32,
    INT8
};

// Helper function to map DataType to C++ type for templated access validation
template<typename T>
constexpr DataType get_cpp_type_data_type() {
    if constexpr (std::is_same_v<T, float>) return DataType::FLOAT32;
    // if constexpr (std::is_same_v<T, <float16_type>>) return DataType::FLOAT16; // Requires a float16 type
    if constexpr (std::is_same_v<T, int32_t>) return DataType::INT32;
    if constexpr (std::is_same_v<T, int8_t>) return DataType::INT8;
    throw std::runtime_error("Unsupported C++ type for tensor data type mapping.");
}

class Tensor {
public:
    Tensor(DataType dtype, const std::vector<int64_t>& shape);
    ~Tensor();

    DataType get_dtype() const;
    const std::vector<int64_t>& get_shape() const;
    
    // Raw data access (void* for interoperability)
    void* get_data();
    const void* get_data() const;
    
    // Templated type-safe data access
    template<typename T>
    T* data() {
        if (get_cpp_type_data_type<T>() != _dtype) {
            throw std::runtime_error("Attempted to access tensor data with incorrect type.");
        }
        return reinterpret_cast<T*>(_data.data());
    }

    template<typename T>
    const T* data() const {
        if (get_cpp_type_data_type<T>() != _dtype) {
            throw std::runtime_error("Attempted to access tensor data with incorrect type.");
        }
        return reinterpret_cast<const T*>(_data.data());
    }

    size_t get_size_in_bytes() const;


private:
    DataType _dtype;
    std::vector<int64_t> _shape;
    std::vector<char> _data; // Raw data buffer
};

} // namespace speedforce
