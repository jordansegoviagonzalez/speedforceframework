import speedforce

def test_engine_instantiation():
    engine = speedforce.engine.Engine()
    assert isinstance(engine, speedforce.engine.Engine)

def test_engine_load_model():
    model = speedforce.load("test_model_for_python.onnx")
    assert isinstance(model, speedforce.engine.Model)
    assert model.name == "dummy_model"
    assert model.path == "test_model_for_python.onnx"

def test_engine_generate():
    engine = speedforce.engine.Engine()
    # Need to load a model first, even if it's a dummy
    model = engine.load_model("another_test_model.onnx") 
    result = engine.generate("hello from python")
    assert "Generated response for 'hello from python'" in result

