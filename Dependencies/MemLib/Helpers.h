#pragma once

#include <cstdint>
#include <bit>

#define AS_FIELD(type, name, fn) __declspec(property(get = fn, put = set##name)) type name
#define BUILD_ACCESS(ptr, type, name, offset)                                                                        \
AS_FIELD(type, name, get##name);                                                                                     \
type get##name() const { return MemLib::direct_access<type>(ptr, offset); }                                                     \
void set##name(type v) const { MemLib::direct_access<type>(ptr, offset) = v; }

namespace MemLib {
    namespace details { // ty brady :)
        template<typename>
        struct resolve_func;

        template<typename R, typename... Args>
        struct resolve_func<R(Args...)> {

            template<typename T>
            struct memfn : std::type_identity<R(T::*)(Args...)> {};

            template<typename T>
            struct memfn<const T> : std::type_identity<R(T::*)(Args...) const> {};

            template<typename T>
            using memfn_t = typename memfn<T>::type;
        };

        template<typename R, typename T, typename... Args>
        struct resolve_func<R(T::*)(Args...)> {
            template<typename>
            using memfn_t = R(T::*)(Args...);
        };

        template<typename R, typename T, typename... Args>
        struct resolve_func<R(T::*)(Args...) const> {
            template<typename>
            using memfn_t = R(T::*)(Args...) const;
        };
    }

    template<typename Fn, typename Inst, typename... Args>
    decltype(auto) call_member_func(Inst* instance, const uintptr_t addr, Args&&... args) {
        using memfn_t = typename details::resolve_func<Fn>::template memfn_t<Inst>;

        const auto memfn = std::bit_cast<memfn_t>(addr);
        return (instance->*memfn)(std::forward<Args>(args)...);
    }

    template<size_t Index, typename Fn, typename Inst, typename... Args>
    decltype(auto) call_vfunc(Inst* instance, Args&&... args) {
        const auto vtable = *reinterpret_cast<uintptr_t* const*>(instance);

        return call_member_func<Fn>(instance, vtable[Index], std::forward<Args>(args)...);
    }

    template <typename R, typename... Args>
    static inline R call_func(void* func, Args... args) {
        return ((R(*)(Args...))func)(args...);
    }

    template <typename Ret, typename... Args>
    static auto call_vfunc_index(uint32_t index, void* thisptr, Args... argList) -> Ret {
        using Fn = Ret(__thiscall*)(void*, Args...);
        return (*static_cast<Fn**>(thisptr))[index](thisptr, std::forward<Args>(argList)...);
    }

    template <typename Ret, typename... Args>
    static inline auto* get_fastcall(void* Original) {
        using Fn = Ret(__fastcall*)(Args...);
        return reinterpret_cast<Fn>(Original);
    }

    template <typename Ret, typename Type>
    static Ret& direct_access(Type* type, size_t offset) {
        union {
            size_t raw;
            Type* source;
            Ret* target;
        } u;

        u.source = type;
        u.raw += offset;

        return *u.target;
    }

    template<typename Ret, typename Type>
    auto& member_at(Type* ptr, int offset) {
        return *reinterpret_cast<Ret*>(reinterpret_cast<std::uintptr_t>(ptr) + offset);
    }

    static uintptr_t offset_from_signature(uintptr_t sig, int offset) {
        return sig + offset + 4 + *reinterpret_cast<int*>(sig + offset);
    }

    template<typename Ret>
    static Ret get_offset_from_signature(const uintptr_t sig, const int offset) {
        return reinterpret_cast<Ret>(offset_from_signature(sig, offset));
    }
};