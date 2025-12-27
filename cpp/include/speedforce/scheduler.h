#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <future>
#include <memory>
#include <atomic>

#include "tensor.h"

namespace speedforce {

// Enum to track the state of a request
enum class RequestStatus {
    WAITING,
    IN_PROGRESS,
    COMPLETED,
    CANCELLED
};

// Represents a single inference request with its state and result promise.
struct InferenceRequest {
    uint64_t id{};
    Tensor input;
    RequestStatus status{RequestStatus::WAITING};
    std::promise<Tensor> promise{};
    std::chrono::high_resolution_clock::time_point received_time{};

    InferenceRequest(uint64_t id_, const Tensor& input_)
        : id(id_),
          input(input_),
          status(RequestStatus::WAITING),
          received_time(std::chrono::high_resolution_clock::now()) {}

    InferenceRequest(uint64_t id_, Tensor&& input_)
        : id(id_),
          input(std::move(input_)),
          status(RequestStatus::WAITING),
          received_time(std::chrono::high_resolution_clock::now()) {}

    InferenceRequest() = delete;
};

// A batch of requests currently being processed by the engine.
// The engine worker will lock this, run inference, and update request statuses.
struct ActiveBatch {
    std::mutex mtx;
    std::vector<std::shared_ptr<InferenceRequest>> requests;
};

class Scheduler {
public:
    Scheduler(size_t max_batch_size = 8, std::chrono::microseconds batch_timeout = std::chrono::microseconds(1000));
    ~Scheduler();

    // Starts the scheduler's background thread.
    void start();

    // Stops the scheduler's background thread.
    void stop();

    // Asynchronously adds a request to the scheduler and returns a future for the result.
    std::future<Tensor> add_request(const Tensor& input);

    // Provides access to the active batch for the engine worker.
    ActiveBatch& get_active_batch();

private:
    // The main loop for the scheduler thread.
    void scheduler_loop();

    // Moves requests from the waiting queue to the active batch.
    void dispatch_requests();

    std::queue<std::shared_ptr<InferenceRequest>> _waiting_queue;
    std::mutex _queue_mutex;
    std::condition_variable _cv;

    ActiveBatch _active_batch;

    size_t _max_batch_size;
    std::chrono::microseconds _batch_timeout;

    std::thread _scheduler_thread;
    std::atomic<bool> _running;
    std::atomic<uint64_t> _next_request_id;
};

} // namespace speedforce