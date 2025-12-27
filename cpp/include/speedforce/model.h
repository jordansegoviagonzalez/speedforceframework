#pragma once

#include <string>
#include <vector>
#include <any>

namespace speedforce {

class Model {
public:
    Model(const std::string& name, const std::string& path);
    ~Model();

    const std::string& get_name() const;
    const std::string& get_path() const;

    void set_backend_handle(std::any handle);
    const std::any& get_backend_handle() const;

private:
    std::string _name;
    std::string _path;
    std::any _backend_handle;
    // Placeholder for model weights and metadata
    std::vector<char> _weights; 
};

} // namespace speedforce