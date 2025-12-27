#include "../include/speedforce/engine.h"
#include "../include/speedforce/backends/backend.h"
#include "../backends/onnxruntime/ort_backend.h" // Include the concrete ORT backend
#include "../backends/tensorrt/trt_backend.h"    // Include the concrete TRT backend
#include "../include/speedforce/model.h"
#include "../include/speedforce/tensor.h"

#include <iostream>
#include <memory>
#include <stdexcept> // For std::runtime_error

namespace speedforce {

Engine::Engine(backends::BackendType preferred_backend) {
    _backend = nullptr; // Initialize to nullptr

    // Attempt to load preferred backend first
    if (preferred_backend == backends::BackendType::TENSOR_RT) {
        try {
            _backend = std::make_unique<backends::TRTBackend>();
            std::cout << "Engine initialized with TensorRT backend." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not initialize TensorRT backend: " << e.what() << std::endl;
            std::cout << "Attempting to initialize ONNX Runtime backend as fallback." << std::endl;
        }
    }

    if (!_backend) { // If TRT backend failed or ONNX_RUNTIME was preferred
        try {
            _backend = std::make_unique<backends::ORTBackend>();
            std::cout << "Engine initialized with ONNX Runtime backend." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: Could not initialize ONNX Runtime backend: " << e.what() << std::endl;
            throw std::runtime_error("Failed to initialize any available backend.");
        }
    }

    if (!_backend) {
        throw std::runtime_error("Failed to initialize any available backend after attempts.");
    }
}

Engine::~Engine() {
    std::cout << "Engine shut down." << std::endl;
}

std::shared_ptr<Model> Engine::load_model(const std::string& model_path) {
    std::cout << "Engine: Loading model from " << model_path << " via backend." << std::endl;
    if (!_backend) {
        throw std::runtime_error("Backend is not initialized.");
    }
    // The backend returns a unique_ptr, we move it into a shared_ptr for Python interoperability
    // This assumes Model has a public default constructor or can be constructed from its raw pointer.
    // A better approach would be to have the backend return shared_ptr directly or use a factory method.
    return std::shared_ptr<Model>(_backend->load_model(model_path).release());
}

Tensor Engine::run_inference(std::shared_ptr<Model> model, const Tensor& input_tensor) {
    std::cout << "Engine: Running inference on model " << model->get_name() << std::endl;
    if (!_backend) {
        throw std::runtime_error("Backend is not initialized.");
    }
    return _backend->run_inference(*model, input_tensor);
}

} // namespace speedforce