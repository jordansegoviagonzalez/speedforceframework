#pragma once

#include "../../include/speedforce/backends/backend.h"
#include <memory>

// Forward-declare ONNX Runtime types to avoid including the heavy header here
namespace Ort {
    struct Env;
    struct SessionOptions;
}

namespace speedforce {
namespace backends {

class ORTBackend : public Backend {
public:
    ORTBackend();
    ~ORTBackend() override;

    std::unique_ptr<Model> load_model(const std::string& model_path) override;
    Tensor run_inference(const Model& model, const Tensor& input_tensor) override;

private:
    std::unique_ptr<Ort::Env> _ort_env;
    std::unique_ptr<Ort::SessionOptions> _session_options;
};

} // namespace backends
} // namespace speedforce