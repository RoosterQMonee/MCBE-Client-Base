#pragma once

#include <Dependencies/LibHat/libhat/Scanner.hpp>
#include <polyhook2/Detour/x64Detour.hpp>
#include <polyhook2/Enums.hpp>

#include <array>
#include <string_view>
#include <tuple>
#include <optional>
#include <vector>
#include <ranges>
#include <span>
#include <iostream>
#include <concepts>
#include <type_traits>

// ts pmo
#define _SIGPAIR(name, sig) { name, \
    std::vector<hat::signature_element>{hat::compile_signature<sig>().begin(), hat::compile_signature<sig>().end()}}

class MemoryManager {
public:
    template <typename FunctionType>
    struct Hooks {
        template <FunctionType* HookFunc>
        static void CreateHook(void* address) {
            auto detour = std::make_unique<PLH::x64Detour>(reinterpret_cast<uint64_t>(address), HookFunc);
            if (!detour->create()) {
                std::cerr << "Failed to create detour for function at address: " << address << std::endl;
                return;
            }

            m_hooks.push_back(std::move(detour));
        }

        template <void* HookFunc>
        static uint64_t GetOriginal() {
            for (auto& hook : m_hooks) {
                if (hook->getDetourFunction() == reinterpret_cast<uint64_t>(HookFunc)) {
                    return reinterpret_cast<uint64_t>(hook->getOriginal());
                }
            }
            return nullptr;
        }

        template <FunctionType* HookFunc>
        static void DisableHook() {
            for (auto& hook : m_hooks) {
                if (hook->getDetourFunction() == reinterpret_cast<uint64_t>(HookFunc)) {
                    hook->unHook();
                    return;
                }
            }
        }
    };

    template <typename T>
    struct Offsets {
        static inline constexpr std::array<std::tuple<std::string_view, T>, 128> offsets{
            // VTables
            std::make_tuple("MinecraftUIRenderContext::ClientInstance", T{8}),
            std::make_tuple("ClientInstance::GetScreenName", T{268}),
            std::make_tuple("ClientInstance::GetLocalPlayer", T{30}),

            // Offsets
            std::make_tuple("ClientInstance::GuiData", T{0x590}),
            std::make_tuple("ScreenView::VisualTree", T{0x48}),
            std::make_tuple("UIControl::LayerName", T{0x20}),
            std::make_tuple("VisualTree::Root", T{0x8}),
            std::make_tuple("UIControl::SizeConstrains", T{0x40}),
            std::make_tuple("UIControl::ParentRelativePosition", T{0x78})
        };

        static constexpr T GetOffset(std::string_view name) {
            for (const auto& [str, offset] : offsets) {
                if (str == name) {
                    return offset;
                }
            }
            throw std::out_of_range("Offset not found");
        }
    };

    struct Signatures {
        static inline std::vector<std::tuple<std::string_view, std::vector<hat::signature_element>>> signatures{
            _SIGPAIR("FieldOfView", "f3 0f 10 48 ? 48 81 c1"),
            _SIGPAIR("DisplayClientMessage", "40 55 53 56 57 41 56 48 8D AC 24 A0 FE FF FF 48 81 EC 60 02 00 00 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 50 01 00 00 41"),
            _SIGPAIR("SetupAndRender", "48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 98 FD")
        };

        static std::vector<hat::signature_element> GetSignature(std::string_view name) {
            for (const auto& [str, sig] : signatures) {
                if (str == name) {
                    return sig;
                }
            }
            throw std::out_of_range("Signature not found");
        }
    };

private:
    static inline std::vector<std::unique_ptr<PLH::x64Detour>> m_hooks;
};

/*
static consteval auto sigs = MemoryManager::SignatureRegistry<
    _SIGPAIR("FieldOfView", "f3 0f 10 48 ? 48 81 c1"),
    _SIGPAIR("DisplayClientMessage", "40 55 53 56 57 41 56 48 8D AC 24 A0 FE FF FF 48 81 EC 60 02 00 00 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 50 01 00 00 41"),
    _SIGPAIR("SetupAndRender", "48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 98 FD")
>{};

static constexpr auto offsets = MemoryManager::OffsetRegistry<
    // VTables
    _OFFPAIR("MinecraftUIRenderContext::ClientInstance", 8),
    _OFFPAIR("ClientInstance::GetScreenName", 268),
    _OFFPAIR("ClientInstance::GetLocalPlayer", 30),

    // Offsets
    _OFFPAIR("ClientInstance::GuiData", 0x590),
    _OFFPAIR("ScreenView::VisualTree", 0x48),
    _OFFPAIR("UIControl::LayerName", 0x20),
    _OFFPAIR("VisualTree::Root", 0x8),
    _OFFPAIR("UIControl::SizeConstrains", 0x40),
    _OFFPAIR("UIControl::ParentRelativePosition", 0x78)
>{};
*/