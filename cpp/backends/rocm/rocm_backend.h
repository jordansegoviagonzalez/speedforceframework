#pragma once

#include "../../include/speedforce/backends/backend.h"

namespace speedforce {
namespace backends {

class ROCmBackend : public Backend {
public:
    ROCmBackend();
    ~ROCmBackend() override;

    std::unique_ptr<Model> load_model(const std::string& model_path) override;
    Tensor run_inference(const Model& model, const Tensor& input_tensor) override;
};

} // namespace backends
} // namespace speedforce
