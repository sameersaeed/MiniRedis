#pragma once 

#include <unordered_map>
#include <optional>
#include <string>

class KeyValueStore {
public:
    void set(const std::string_view key, std::string_view value);
    std::optional<std::string_view> get(const std::string_view key) const;
    bool remove(const std::string_view key);
    bool exists(const std::string_view key) const;

private:
    std::unordered_map<std::string, std::string> m_values;
};