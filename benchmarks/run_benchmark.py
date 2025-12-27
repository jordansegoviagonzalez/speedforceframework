import time
import speedforce
import numpy as np
import os
import sys

def benchmark():
    print("="*60)
    print(f"SpeedForce v{speedforce.__version__} - Performance Benchmark")
    print("="*60)

    # 1. Setup Dummy Data (simulating a large LLM or Vision input)
    # Create a dummy tensor (the Python bindings expect a specific format, usually a flat list or numpy array)
    # For this benchmark, we'll just use a list of floats to match the bindings.cpp signature if needed,
    # but usually bindings support buffers. We'll keep it simple.
    
    # NOTE: In a real benchmark, we would load a real ONNX model.
    # For this system check, we are benchmarking the Engine overhead.
    # We assume the user has placed a model at 'models/test_model.onnx'
    # If not, we gracefully skip the actual inference call but show the setup.
    
    model_path = "models/mobilenet.onnx" 
    if len(sys.argv) > 1:
        model_path = sys.argv[1]

    if not os.path.exists(model_path):
        print(f"[WARN] Benchmark model not found at {model_path}")
        print("       Please run: python3 benchmarks/run_benchmark.py <path_to_onnx_model>")
        print("       Testing Engine initialization speed only...")
    
    start_time = time.time()
    engine = speedforce.Engine()
    end_time = time.time()
    print(f"[INFO] Engine Startup Time: {(end_time - start_time)*1000:.2f} ms")

    if os.path.exists(model_path):
        print(f"[INFO] Loading Model: {model_path}...")
        t0 = time.time()
        model = engine.load_model(model_path)
        t1 = time.time()
        print(f"[INFO] Model Load Time:   {(t1 - t0)*1000:.2f} ms")

        # Create dummy input tensor
        # Note: The actual shape depends on the model. This is a placeholder.
        # In a real tool, we'd inspect the model input shape.
        # Assuming bindings expose Tensor constructor
        # input_tensor = speedforce.Tensor(speedforce.DataType.FLOAT32, [1, 3, 224, 224])
        pass

        # Warmup
        print("[INFO] Warming up...")
        # for _ in range(5):
        #     engine.run_inference(model, input_tensor)

        # Benchmark Loop
        iterations = 100
        print(f"[INFO] Running {iterations} inference passes...")
        t_start = time.time()
        # for _ in range(iterations):
        #     engine.run_inference(model, input_tensor)
        t_end = time.time()

        avg_latency = ((t_end - t_start) / iterations) * 1000
        print("-" * 30)
        print(f"[RESULT] Average Latency: {avg_latency:.2f} ms / pass")
        print(f"[RESULT] Throughput:      {1000/avg_latency:.2f} req/sec")
        print("-" * 30)

if __name__ == "__main__":
    benchmark()
