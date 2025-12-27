from ._internal import _bindings

class Model:
    """A user-friendly Python wrapper for a C++ SpeedForce Model."""
    def __init__(self, cpp_model: _bindings.Model):
        """
        Initializes the Python Model wrapper.
        
        Note: Users should not create Model objects directly.
        Use speedforce.load(model_path) instead.
        """
        self._cpp_model = cpp_model

    @property
    def name(self) -> str:
        """The name of the loaded model."""
        return self._cpp_model.get_name()

    @property
    def path(self) -> str:
        """The file path of the loaded model."""
        return self._cpp_model.get_path()

    def generate(self, prompt: str) -> str:
        """
        Runs inference on the model.
        
        (This is a high-level abstraction. For now, it simulates creating a tensor
        from the prompt and running inference.)
        """
        global _default_engine
        if _default_engine is None:
            raise RuntimeError("SpeedForce engine has not been initialized.")

        # 1. Simulate tokenization/tensor creation from prompt
        # In a real NLP model, this would be a sophisticated process.
        print(f"Model: Generating response for prompt: '{prompt}'")
        dummy_input_tensor = _bindings.Tensor(_bindings.DataType.FLOAT32, [1, len(prompt)])
        
        # 2. Run inference by calling the engine's method
        output_tensor = _default_engine._cpp_engine.run_inference(self._cpp_model, dummy_input_tensor)

        # 3. Simulate processing the output tensor into a string
        # A real implementation would convert tensor data to text.
        output_shape = output_tensor.get_shape()
        return f"Inference complete. Output tensor shape: {output_shape}"


class Engine:
    """
    The main SpeedForce engine controller. Manages backends and model execution.
    """
    def __init__(self):
        """Initializes the C++ engine bindings."""
        self._cpp_engine = _bindings.Engine()

    def load_model(self, model_path: str) -> Model:
        """
        Loads a model from the given path into the C++ engine and returns
        a Python wrapper for it.
        """
        print(f"Engine: Loading model '{model_path}'...")
        cpp_model = self._cpp_engine.load_model(model_path)
        print("Engine: Model loaded successfully.")
        return Model(cpp_model)

# --- Public API ---

_default_engine = None

def load(model_path: str) -> Model:
    """
    Loads a model into the default SpeedForce engine.
    
    This is the primary entry point for users.
    
    Args:
        model_path: The file path to the model (e.g., 'my_model.onnx').
        
    Returns:
        A Model object ready for inference.
    """
    global _default_engine
    if _default_engine is None:
        print("Initializing default SpeedForce engine...")
        _default_engine = Engine()
    
    return _default_engine.load_model(model_path)