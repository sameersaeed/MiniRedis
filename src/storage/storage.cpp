#include "storage.hpp"

void KeyValueStore::set(const std::string_view key, std::string_view value) {
    m_values.insert_or_assign(std::string(key), std::string(value));
}

std::optional<std::string_view> KeyValueStore::get(const std::string_view key) const {
    auto it = m_values.find(std::string(key));
    if (it != m_values.end()) {
        return it->second;
    }

    return std::nullopt;
}

bool KeyValueStore::remove(const std::string_view key) {
    return m_values.erase(std::string(key)) > 0;
}

bool KeyValueStore::exists(const std::string_view key) const {
    return m_values.contains(std::string(key));
}