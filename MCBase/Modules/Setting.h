#pragma once

#include <variant>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#ifdef max
#undef max
#endif

template<typename T>
struct StorageConstraints {
    static constexpr bool has_min_max = false;
    static constexpr bool has_default_state = false;
};

template<>
struct StorageConstraints<float> {
    static constexpr bool has_min_max = true;
    static constexpr bool has_default_state = false;
    float min_val = std::numeric_limits<float>::lowest();
    float max_val = std::numeric_limits<float>::max();
};

template<>
struct StorageConstraints<std::string> {
    static constexpr bool has_min_max = true;
    static constexpr bool has_default_state = false;
    size_t min_len = 0;
    size_t max_len = std::string::npos;
};

template<>
struct StorageConstraints<bool> {
    static constexpr bool has_min_max = false;
    static constexpr bool has_default_state = true;
    bool default_state = false;
};

template<typename T>
struct Setting {
private:
    std::variant<T> m_value;
    StorageConstraints<T> m_constraints;

public:
    Setting() : m_value(T()) {}

    template<typename U>
    void SetValue(U&& newValue) {
        m_value = std::forward<U>(newValue);
    }

    T GetValue() const {
        return std::get<T>(m_value);
    }

    template<typename MinMaxType>
    void SetMinMax(MinMaxType min_val, MinMaxType max_val) {
        if constexpr (StorageConstraints<T>::has_min_max) {
            m_constraints.min_val = static_cast<decltype(m_constraints.min_val)>(min_val);
            m_constraints.max_val = static_cast<decltype(m_constraints.max_val)>(max_val);
        }
    }

    template<typename DefaultState>
    void SetDefaultState(DefaultState state) {
        if constexpr (StorageConstraints<T>::has_default_state) {
            m_constraints.default_state = static_cast<bool>(state);
        }
    }
};