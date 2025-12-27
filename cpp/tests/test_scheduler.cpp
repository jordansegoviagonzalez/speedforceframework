#include "gtest/gtest.h"
#include "../include/speedforce/scheduler.h"
#include <thread>
#include <chrono>
#include <vector>

using namespace std::chrono_literals;

// Test fixture for the new Scheduler
class ContinuousSchedulerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed before each test
    }

    void TearDown() override {
        // Ensure scheduler is stopped after each test
        scheduler.stop();
    }

    // Using a realistic small batch size for testing
    speedforce::Scheduler scheduler{4, 1000us};
};

TEST_F(ContinuousSchedulerTest, StartAndStop) {
    scheduler.start();
    ASSERT_NO_THROW(scheduler.stop());
}

TEST_F(ContinuousSchedulerTest, AddRequestReturnsValidFuture) {
    scheduler.start();
    speedforce::Tensor input(speedforce::DataType::FLOAT32, {1});
    auto future = scheduler.add_request(input);
    ASSERT_TRUE(future.valid());
}

TEST_F(ContinuousSchedulerTest, BasicBatching) {
    scheduler.start();
    speedforce::Tensor input(speedforce::DataType::FLOAT32, {1});

    // Add two requests
    auto future1 = scheduler.add_request(input);
    auto future2 = scheduler.add_request(input);

    // Give the scheduler thread time to process the queue
    std::this_thread::sleep_for(20ms);

    auto& batch = scheduler.get_active_batch();
    std::lock_guard<std::mutex> lock(batch.mtx);
    
    ASSERT_EQ(batch.requests.size(), 2);
    ASSERT_EQ(batch.requests[0]->status, speedforce::RequestStatus::IN_PROGRESS);
    ASSERT_EQ(batch.requests[1]->status, speedforce::RequestStatus::IN_PROGRESS);
}

TEST_F(ContinuousSchedulerTest, RequestIsFulfilled) {
    scheduler.start();
    speedforce::Tensor input(speedforce::DataType::FLOAT32, {1});
    auto future = scheduler.add_request(input);

    // Give scheduler time to dispatch
    std::this_thread::sleep_for(20ms);

    // Simulate the engine processing the request
    auto& batch = scheduler.get_active_batch();
    {
        std::lock_guard<std::mutex> lock(batch.mtx);
        ASSERT_FALSE(batch.requests.empty());
        
        speedforce::Tensor output(speedforce::DataType::FLOAT32, {1});
        output.get_data<float>()[0] = 42.0f; // Dummy result
        
        // Fulfill the promise and mark as complete
        batch.requests[0]->promise.set_value(std::move(output));
        batch.requests[0]->status = speedforce::RequestStatus::COMPLETED;
    }

    // The future should now be ready and contain the value
    auto result_tensor = future.get();
    ASSERT_EQ(result_tensor.get_data<float>()[0], 42.0f);

    // Give scheduler time to clean up the completed request
    std::this_thread::sleep_for(20ms);

    {
        std::lock_guard<std::mutex> lock(batch.mtx);
        ASSERT_TRUE(batch.requests.empty()); // Batch should be empty now
    }
}

TEST_F(ContinuousSchedulerTest, ContinuousBatchingFillsSlots) {
    scheduler.start(); // Max batch size is 4 for this test fixture
    speedforce::Tensor input(speedforce::DataType::FLOAT32, {1});

    // Add 4 requests to fill the batch
    std::vector<std::future<speedforce::Tensor>> futures;
    for(int i=0; i<4; ++i) {
        futures.push_back(scheduler.add_request(input));
    }

    // Give scheduler time to dispatch
    std::this_thread::sleep_for(20ms);

    {
        auto& batch = scheduler.get_active_batch();
        std::lock_guard<std::mutex> lock(batch.mtx);
        ASSERT_EQ(batch.requests.size(), 4);
    }
    
    // Add a 5th request, which should be waiting
    auto future5 = scheduler.add_request(input);
    std::this_thread::sleep_for(10ms); // give time to queue

    {
        auto& batch = scheduler.get_active_batch();
        std::lock_guard<std::mutex> lock(batch.mtx);
        ASSERT_EQ(batch.requests.size(), 4); // Still 4
    }

    // Simulate the engine finishing two requests
    auto& batch = scheduler.get_active_batch();
     {
        std::lock_guard<std::mutex> lock(batch.mtx);
        speedforce::Tensor output(speedforce::DataType::FLOAT32, {1});
        batch.requests[0]->promise.set_value(output);
        batch.requests[0]->status = speedforce::RequestStatus::COMPLETED;
        batch.requests[1]->promise.set_value(output);
        batch.requests[1]->status = speedforce::RequestStatus::COMPLETED;
    }

    // Give the scheduler time to clean up and dispatch the waiting request
    std::this_thread::sleep_for(50ms);

    // The active batch should now contain 3 requests: 2 old and 1 new
    {
        std::lock_guard<std::mutex> lock(batch.mtx);
        ASSERT_EQ(batch.requests.size(), 3);
        // The 5th request should now be in the batch
        bool found_fifth = false;
        for(const auto& req : batch.requests) {
            if (req->id == 4) { // IDs are 0-based
                found_fifth = true;
                break;
            }
        }
        ASSERT_TRUE(found_fifth);
    }
}
