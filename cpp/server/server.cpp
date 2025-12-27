#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <cstdlib> // For std::getenv

#include <grpcpp/grpcpp.h>
#include "service_impl.h"
#include "../../include/speedforce/engine.h"
#include "../../include/speedforce/tensor.h"
#include "../../include/speedforce/utils/logger.h"

// Define version if not provided by CMake
#ifndef SPEEDFORCE_VERSION
#define SPEEDFORCE_VERSION "0.0.0-dev"
#endif

namespace speedforce {
namespace server {

SpeedForceServiceImpl::SpeedForceServiceImpl(speedforce::Engine& engine) : _engine(engine) {}

grpc::Status SpeedForceServiceImpl::HealthCheck(grpc::ServerContext* context,
                                                const HealthCheckRequest* request,
                                                HealthCheckResponse* response) {
    // In a real scenario, we might check if the engine is responsive or if GPUs are healthy.
    // For now, if the server responds, it is healthy.
    response->set_status(HealthCheckResponse::SERVING);
    return grpc::Status::OK;
}

grpc::Status SpeedForceServiceImpl::LoadModel(grpc::ServerContext* context, 
                                               const LoadModelRequest* request, 
                                               LoadModelResponse* response) {
    SF_LOG_INFO("Received LoadModel request for path: ", request->model_path());
    
    try {
        speedforce::Model model = _engine.load_model(request->model_path());
        std::string model_id = model.get_name(); // Using model name as ID for now
        _loaded_models[model_id] = model; // Store the loaded model
        response->set_model_id(model_id);
        response->set_success(true);
        response->set_message("Model loaded successfully.");
        SF_LOG_INFO("Model loaded successfully with ID: ", model_id);
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        std::string error_msg = std::string("Failed to load model: ") + e.what();
        SF_LOG_ERROR(error_msg);
        response->set_model_id("");
        response->set_success(false);
        response->set_message(error_msg);
        return grpc::Status::CANCELLED;
    }
}

grpc::Status SpeedForceServiceImpl::Infer(grpc::ServerContext* context, 
                                          const InferenceRequest* request, 
                                          InferenceResponse* response) {
    SF_LOG_DEBUG("Received Infer request for model_id: ", request->model_id());

    auto it = _loaded_models.find(request->model_id());
    if (it == _loaded_models.end()) {
        std::string error_msg = "Model not found. Please load the model first.";
        SF_LOG_WARN(error_msg);
        response->set_success(false);
        response->set_message(error_msg);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Model not found");
    }
    speedforce::Model& model = it->second;

    // Basic string-to-tensor conversion (assuming comma-separated floats)
    std::vector<float> input_data;
    std::stringstream ss(request->input_data());
    std::string item;
    while (std::getline(ss, item, ',')) {
        try {
            input_data.push_back(std::stof(item));
        } catch (...) {
             SF_LOG_ERROR("Invalid input data format.");
             return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid input data");
        }
    }
    speedforce::Tensor input_tensor(speedforce::DataType::FLOAT32, {1, (uint64_t)input_data.size()});
    std::copy(input_data.begin(), input_data.end(), input_tensor.get_data<float>());

    try {
        speedforce::Tensor output_tensor = _engine.run_inference(model, input_tensor);
        
        // Basic tensor-to-string conversion
        std::stringstream out_ss;
        const float* output_ptr = output_tensor.get_data<float>();
        for (size_t i = 0; i < output_tensor.get_shape()[1]; ++i) {
            out_ss << output_ptr[i] << (i == output_tensor.get_shape()[1] - 1 ? "" : ",");
        }

        response->set_model_id(request->model_id());
        response->set_output_data(out_ss.str());
        response->set_success(true);
        response->set_message("Inference completed successfully.");
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        std::string error_msg = std::string("Failed to run inference: ") + e.what();
        SF_LOG_ERROR(error_msg);
        response->set_model_id(request->model_id());
        response->set_output_data("");
        response->set_success(false);
        response->set_message(error_msg);
        return grpc::Status::CANCELLED;
    }
}

void RunServer() {
    // Configuration via Environment Variables
    const char* port_env = std::getenv("SF_PORT");
    std::string port = port_env ? port_env : "50051";
    std::string server_address("0.0.0.0:" + port);

    speedforce::Engine engine;
    SpeedForceServiceImpl service(engine);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    
    SF_LOG_INFO("SpeedForce v", SPEEDFORCE_VERSION, " Server listening on ", server_address);
    server->Wait();
}

} // namespace server
} // namespace speedforce

int main() {
    speedforce::server::RunServer();
    return 0;
}
