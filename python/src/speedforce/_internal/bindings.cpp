#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // For std::vector and std::string

#include "../../../cpp/include/speedforce/engine.h"
#include "../../../cpp/include/speedforce/model.h"
#include "../../../cpp/include/speedforce/tensor.h"

namespace py = pybind11;
using namespace speedforce;

// By default, pybind11 uses unique_ptr for class bindings. For shared ownership
// between C++ and Python, we need to declare the holder type as std::shared_ptr.
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_bindings, m) {
    m.doc() = "pybind11 bindings for the SpeedForce C++ library";

    // Bind DataType enum
    py::enum_<DataType>(m, "DataType")
        .value("FLOAT32", DataType::FLOAT32)
        .value("FLOAT16", DataType::FLOAT16)
        .value("INT32", DataType::INT32)
        .value("INT8", DataType::INT8)
        .export_values();

    // Bind Tensor class
    py::class_<Tensor>(m, "Tensor")
        .def(py::init<DataType, const std::vector<int64_t>&>(),
             py::arg("dtype"), py::arg("shape"))
        .def("get_dtype", &Tensor::get_dtype)
        .def("get_shape", &Tensor::get_shape)
        .def("get_size_in_bytes", &Tensor::get_size_in_bytes)
        // For simplicity, not exposing raw data buffer directly yet
        ;

    // Bind Model class. We bind it with std::shared_ptr as the holder type
    // because the Engine returns shared_ptrs. Python's garbage collector
    // will now correctly manage the object's lifecycle with C++.
    py::class_<Model, std::shared_ptr<Model>>(m, "Model")
        // We don't expose a constructor because Models should only be created via Engine::load_model
        .def("get_name", &Model::get_name)
        .def("get_path", &Model::get_path)
        ;

    // Bind Engine class
    py::class_<Engine>(m, "Engine")
        .def(py::init<>())
        .def("load_model", &Engine::load_model, 
             py::arg("model_path"), 
             py::return_value_policy::take_ownership,
             "Loads a model and returns a shared pointer to it.")
        .def("run_inference", &Engine::run_inference, 
             py::arg("model"), py::arg("input_tensor"),
             "Runs inference on a given model.")
        ;
}