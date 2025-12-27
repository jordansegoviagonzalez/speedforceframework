#pragma once

#include <string>
#include <memory>
#include <vector>
#include "speedforce/model.h"
#include "speedforce/tensor.h"

namespace speedforce {
namespace backends {

// Enum to specify the desired backend type
enum class BackendType {
    ONNX_RUNTIME,
    TENSOR_RT,
    // Add other backend types here as they are implemented (e.g., ROCM)
};

class Backend {
public:
    virtual ~Backend() = default;

    // Pure virtual function to load a model.
    virtual std::unique_ptr<Model> load_model(const std::string& model_path) = 0;

    // Pure virtual function to run inference.
    virtual Tensor run_inference(const Model& model, const Tensor& input_tensor) = 0;
};

} // namespace backends
} // namespace speedforce
