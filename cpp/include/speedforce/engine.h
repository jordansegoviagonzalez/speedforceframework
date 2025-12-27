#pragma once

#include <memory>
#include <vector>

#include "model.h"
#include "tensor.h"
#include "backends/backend.h" // Include for BackendType

namespace speedforce {

namespace backends {
class Backend; // Forward declaration
}

class Engine {
public:
    // Constructor that optionally takes a BackendType to specify the backend.
    // If no type is provided, the engine will attempt to auto-select the best available backend.
    explicit Engine(backends::BackendType preferred_backend = backends::BackendType::ONNX_RUNTIME);
    ~Engine();

    // Load a model into the engine. Returns a shared_ptr to the model.
    std::shared_ptr<Model> load_model(const std::string& model_path);

    // Run inference on a loaded model.
    Tensor run_inference(std::shared_ptr<Model> model, const Tensor& input_tensor);

private:
    std::unique_ptr<backends::Backend> _backend;
};

} // namespace speedforce