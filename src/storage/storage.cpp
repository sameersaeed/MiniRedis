#include "storage.hpp"

void KeyValueStore::set(const std::string_view key, std::string_view value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_values.insert_or_assign(std::string(key), std::string(value));
}

std::optional<std::string> KeyValueStore::get(const std::string_view key) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_values.find(std::string(key));
    if (it != m_values.end()) {
        return it->second;
    }

    return std::nullopt;
}

bool KeyValueStore::remove(const std::string_view key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_values.erase(std::string(key)) > 0;
}

bool KeyValueStore::exists(const std::string_view key) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_values.contains(std::string(key));
}