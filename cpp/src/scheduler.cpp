#include "../include/speedforce/scheduler.h"
#include <iostream>
#include <vector>
#include <algorithm>

namespace speedforce {

Scheduler::Scheduler(size_t max_batch_size, std::chrono::microseconds batch_timeout)
    : _max_batch_size(max_batch_size), 
      _batch_timeout(batch_timeout),
      _running(false),
      _next_request_id(0) {
    std::cout << "Continuous Batching Scheduler initialized." << std::endl;
}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    if (_running) return;
    _running = true;
    _scheduler_thread = std::thread(&Scheduler::scheduler_loop, this);
    std::cout << "Scheduler started." << std::endl;
}

void Scheduler::stop() {
    if (!_running) return;
    _running = false;
    _cv.notify_all(); // Wake up the scheduler thread so it can exit
    if (_scheduler_thread.joinable()) {
        _scheduler_thread.join();
    }
    std::cout << "Scheduler stopped." << std::endl;
}

std::future<Tensor> Scheduler::add_request(const Tensor& input) {
    const uint64_t id = _next_request_id.fetch_add(1, std::memory_order_relaxed);
    auto request = std::make_shared<InferenceRequest>(id, input);
    
    std::future<Tensor> future = request->promise.get_future();

    {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        _waiting_queue.push(std::move(request));
    }
    
    _cv.notify_one();
    return future;
}

ActiveBatch& Scheduler::get_active_batch() {
    return _active_batch;
}

void Scheduler::scheduler_loop() {
    while (_running) {
        {
            // Lock the active batch to clean up finished requests
            std::unique_lock<std::mutex> active_lock(_active_batch.mtx);
            
            // Remove completed requests from the active batch
            auto it = std::remove_if(_active_batch.requests.begin(), _active_batch.requests.end(),
                [](const auto& req) {
                    if (req->status == RequestStatus::COMPLETED) {
                        // The engine is responsible for setting the promise value
                        return true;
                    }
                    if (req->status == RequestStatus::CANCELLED) {
                        req->promise.set_exception(std::make_exception_ptr(std::runtime_error("Request cancelled")));
                        return true;
                    }
                    return false;
                });
            
            if (it != _active_batch.requests.end()) {
                _active_batch.requests.erase(it, _active_batch.requests.end());
                // After cleaning up, we might have space, so we can notify ourselves to dispatch immediately
                _cv.notify_one(); 
            }
        } // Release active_batch lock

        dispatch_requests();

        // Wait for a notification or a timeout
        std::unique_lock<std::mutex> lock(_queue_mutex);
        _cv.wait_for(lock, _batch_timeout, [this] {
            return !_waiting_queue.empty() || !_running;
        });
    }
}

void Scheduler::dispatch_requests() {
    std::unique_lock<std::mutex> queue_lock(_queue_mutex);
    if (_waiting_queue.empty()) {
        return;
    }

    std::unique_lock<std::mutex> active_lock(_active_batch.mtx);
    while (!_waiting_queue.empty() && _active_batch.requests.size() < _max_batch_size) {
        auto& request = _waiting_queue.front();
        request->status = RequestStatus::IN_PROGRESS;
        _active_batch.requests.push_back(std::move(request));
        _waiting_queue.pop();
    }
}

} // namespace speedforce