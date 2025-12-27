#pragma once

#include <grpcpp/grpcpp.h>
// Include the generated header. Path depends on build, but usually:
#include "service.grpc.pb.h"
#include "service.pb.h"

#include "../../include/speedforce/engine.h"
#include <map>
#include <string>

namespace speedforce {
namespace server {

class SpeedForceServiceImpl final : public SpeedForceService::Service {
public:
    explicit SpeedForceServiceImpl(speedforce::Engine& engine);

    grpc::Status HealthCheck(grpc::ServerContext* context,
                             const HealthCheckRequest* request,
                             HealthCheckResponse* response) override;

    grpc::Status LoadModel(grpc::ServerContext* context,
                           const LoadModelRequest* request,
                           LoadModelResponse* response) override;

    grpc::Status Infer(grpc::ServerContext* context,
                       const InferenceRequest* request,
                       InferenceResponse* response) override;

private:
    speedforce::Engine& _engine;
    std::map<std::string, speedforce::Model> _loaded_models;
};

} // namespace server
} // namespace speedforce

