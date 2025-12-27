#include "gtest/gtest.h"
#include "../include/speedforce/engine.h"
#include "../include/speedforce/model.h"
#include "../include/speedforce/tensor.h"

TEST(EngineTest, CanInstantiateEngine) {
    speedforce::Engine engine;
    // If no exceptions are thrown, instantiation is considered successful
    SUCCEED();
}

TEST(EngineTest, CanLoadModel) {
    speedforce::Engine engine;
    speedforce::Model model = engine.load_model("test_model.onnx");
    ASSERT_EQ(model.get_name(), "dummy_model"); // Based on current placeholder in engine.cpp
    ASSERT_EQ(model.get_path(), "test_model.onnx");
}

TEST(EngineTest, CanRunInference) {
    speedforce::Engine engine;
    speedforce::Model model("test_model", "test_model.onnx");
    speedforce::Tensor input_tensor(speedforce::DataType::FLOAT32, {1, 10});
    
    speedforce::Tensor output_tensor = engine.run_inference(model, input_tensor);
    // Based on current placeholder in engine.cpp, it returns a dummy tensor
    ASSERT_EQ(output_tensor.get_dtype(), speedforce::DataType::FLOAT32);
    ASSERT_EQ(output_tensor.get_shape().size(), 1);
    ASSERT_EQ(output_tensor.get_shape()[0], 1);
}
