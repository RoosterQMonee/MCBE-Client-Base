#pragma once

#include <vector>
#include <memory>
#include <ranges>


template<class T>
class Manager {
private:
    std::vector<std::unique_ptr<T>> m_items;

public:
    Manager() = default;
    ~Manager() = default;

    template<class U, typename... Args>
    U* Add(Args&&... args) {
        auto ptr = std::make_unique<U>(std::forward<Args>(args)...);
        auto* raw = ptr.get();
        m_items.push_back(std::move(ptr));
        return raw;
    }

    template<class... U>
    void AddMultiple() {
        (this->Add<U>(), ...);
    };

    template<std::invocable<T> F>
    void ForEach(F&& callback) const {
        for (const auto& item : m_items) {
            callback(*item.get());  // Dereference the pointer
        }
    }

    template<std::invocable<T*> F>
    auto Filter(F&& callback) const {
        return m_items | std::views::filter([&callback](const auto& ptr) {
            return callback(ptr.get());
        });
    }
};