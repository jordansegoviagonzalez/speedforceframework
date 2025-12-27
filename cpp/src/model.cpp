#include "../include/speedforce/model.h"
#include <iostream>

namespace speedforce {

Model::Model(const std::string& name, const std::string& path)
    : _name(name), _path(path) {
    std::cout << "Model: Initialized model '" << _name << "' from path '" << _path << "'" << std::endl;
    // _weights is currently a placeholder. If raw model bytes are needed directly within
    // the Model object (e.g., for serialization), they would be loaded here.
    // However, typically backends manage their own model representations internally.
}

Model::~Model() {
    std::cout << "Model: Destroyed model '" << _name << "'" << std::endl;
}

const std::string& Model::get_name() const {
    return _name;
}

const std::string& Model::get_path() const {
    return _path;
}

void Model::set_backend_handle(std::any handle) {
    _backend_handle = handle;
}

const std::any& Model::get_backend_handle() const {
    return _backend_handle;
}

} // namespace speedforce
