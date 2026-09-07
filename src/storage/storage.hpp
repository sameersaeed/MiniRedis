#pragma once 

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

class KeyValueStore {
public:
    void set(const std::string_view key, std::string_view value);
    std::optional<std::string> get(const std::string_view key) const;
    bool remove(const std::string_view key);
    bool exists(const std::string_view key) const;

private:
    std::unordered_map<std::string, std::string> m_values;
    mutable std::mutex m_mutex;
};