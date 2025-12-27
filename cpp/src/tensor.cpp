#include "../include/speedforce/tensor.h"
#include <numeric> // For std::accumulate
#include <stdexcept> // For std::runtime_error

namespace speedforce {

// Helper function to get the size of a single element in bytes based on DataType
static size_t get_element_size(DataType dtype) {
    switch (dtype) {
        case DataType::FLOAT32: return 4;
        case DataType::FLOAT16: return 2;
        case DataType::INT32: return 4;
        case DataType::INT8: return 1;
        default:
            throw std::runtime_error("Unknown DataType");
    }
}

Tensor::Tensor(DataType dtype, const std::vector<int64_t>& shape)
    : _dtype(dtype), _shape(shape) {
    
    size_t num_elements = 1;
    for (int64_t dim : shape) {
        if (dim <= 0) {
            throw std::runtime_error("Tensor dimensions must be positive.");
        }
        num_elements *= dim;
    }

    size_t element_size = get_element_size(dtype);
    _data.resize(num_elements * element_size);
}

Tensor::~Tensor() {
    // Destructor
}

DataType Tensor::get_dtype() const {
    return _dtype;
}

const std::vector<int64_t>& Tensor::get_shape() const {
    return _shape;
}

void* Tensor::get_data() {
    return _data.data();
}

const void* Tensor::get_data() const {
    return _data.data();
}

size_t Tensor::get_size_in_bytes() const {
    return _data.size();
}

} // namespace speedforce
