#include "ort_backend.h"
#include <iostream>
#include <stdexcept>
#include "onnxruntime_cxx_api.h"

// Helper to convert our DataType to ONNX's ONNXTensorElementDataType
ONNXTensorElementDataType to_ort_type(speedforce::DataType dtype) {
    switch (dtype) {
        case speedforce::DataType::FLOAT32: return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
        case speedforce::DataType::INT32:   return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32;
        case speedforce::DataType::INT8:    return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8;
        // Add other types as needed
        default: throw std::runtime_error("Unsupported data type for ONNX Runtime");
    }
}


namespace speedforce {
namespace backends {

ORTBackend::ORTBackend() {
    // 1. Initialize ONNX Runtime environment
    _ort_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "speedforce_ort_backend");

    // 2. Configure session options
    _session_options = std::make_unique<Ort::SessionOptions>();
    _session_options->SetIntraOpNumThreads(1);
    _session_options->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    std::cout << "ONNX Runtime Backend initialized." << std::endl;
}

ORTBackend::~ORTBackend() {
    std::cout << "ONNX Runtime Backend shut down." << std::endl;
}

std::unique_ptr<Model> ORTBackend::load_model(const std::string& model_path) {
    std::cout << "ONNX Runtime Backend: Loading model from " << model_path << std::endl;

    // 1. Create an Ort::Session for the given model_path
    auto session = std::make_shared<Ort::Session>(*_ort_env, model_path.c_str(), *_session_options);

    // 2. Create a SpeedForce Model object
    auto model = std::make_unique<Model>(model_path, model_path);

    // 3. Store the session object in the Model's backend handle
    model->set_backend_handle(session);

    std::cout << "ONNX Runtime Backend: Model loaded successfully." << std::endl;
    return model;
}

Tensor ORTBackend::run_inference(const Model& model, const Tensor& input_tensor) {
    // 1. Retrieve the Ort::Session associated with the 'model' object
    std::shared_ptr<Ort::Session> session;
    try {
        session = std::any_cast<std::shared_ptr<Ort::Session>>(model.get_backend_handle());
    } catch (const std::bad_any_cast& e) {
        throw std::runtime_error("Failed to cast backend handle to Ort::Session. Model may not have been loaded correctly by the ONNX Runtime backend.");
    }

    if (!session) {
        throw std::runtime_error("The ONNX Runtime session is null. Model may not have been loaded correctly.");
    }
    
    // 2. Get allocator and model input/output details
    Ort::AllocatorWithDefaultOptions allocator;

    // For simplicity, we assume the model has one input and one output.
    // A production-ready implementation would handle multiple inputs/outputs.
    if (session->GetInputCount() != 1 || session->GetOutputCount() != 1) {
        throw std::runtime_error("This version only supports single-input, single-output models.");
    }

    // Get input details
    auto input_name = session->GetInputNameAllocated(0, allocator);
    const char* input_name_ptr = input_name.get();
    Ort::TypeInfo input_type_info = session->GetInputTypeInfo(0);
    auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
    std::vector<int64_t> input_shape = input_tensor_info.GetShape();

    // Get output details
    auto output_name = session->GetOutputNameAllocated(0, allocator);
    const char* output_name_ptr = output_name.get();


    // 3. Create input Ort::Value from our speedforce::Tensor
    // Check for dynamic shapes (any dimension is -1) - this is a simplified check
    for(size_t i = 0; i < input_shape.size(); ++i) {
        if(input_shape[i] == -1) {
            input_shape[i] = input_tensor.get_shape()[i];
        }
    }

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_ort_value = Ort::Value::CreateTensor(
        memory_info,
        const_cast<void*>(input_tensor.get_data()), 
        input_tensor.get_size_in_bytes(),
        input_shape.data(), 
        input_shape.size(),
        to_ort_type(input_tensor.get_dtype())
    );

    // 4. Run the session
    std::vector<Ort::Value> output_tensors = session->Run(
        Ort::RunOptions{nullptr}, 
        &input_name_ptr, &input_ort_value, 1, 
        &output_name_ptr, 1
    );

    // 5. Convert the output Ort::Value back to our speedforce::Tensor
    if (output_tensors.empty() || !output_tensors[0].IsTensor()) {
        throw std::runtime_error("Inference did not produce a valid output tensor.");
    }

    Ort::Value& output_ort_value = output_tensors[0];
    auto output_tensor_info = output_ort_value.GetTensorTypeAndShapeInfo();
    
    // Create the speedforce output tensor
    Tensor result_tensor(output_tensor_info.GetElementType(), output_tensor_info.GetShape());

    // Copy the data
    const void* ort_data = output_ort_value.GetTensorData<void>();
    void* result_data = result_tensor.get_data();
    memcpy(result_data, ort_data, result_tensor.get_size_in_bytes());

    return result_tensor;
}

} // namespace backends
} // namespace speedforce