#pragma once

#include <Windows.h>
#include <intrin.h>
#include <immintrin.h>
#include <cstring>
#include <optional>
#include <array>
#include <vector>
#include <string>
#include <string_view>

#include "Memory.h"
#include <execution>


namespace MemLib {
    enum class InstructionSet {
        NONE,
        SSE,
        AVX2,
        AVX512
    };

    struct ScanResult {
        std::string_view signature_name;
        uintptr_t address;
        size_t module_base;
        size_t module_size;
    };

    class Scanner {
    private:
        static InstructionSet detect_instruction_set() {
            int cpuInfo[4];
            __cpuid(cpuInfo, 7);

            if (cpuInfo[1] & (1 << 25)) return InstructionSet::SSE; // prioritze SSE :)
            if (cpuInfo[1] & (1 << 16)) return InstructionSet::AVX512;
            if (cpuInfo[1] & (1 << 5)) return InstructionSet::AVX2;
            return InstructionSet::NONE;
        }

    private:
        static void* allocate_aligned(size_t size, size_t alignment) {
            return memoryManager.allocate(size, alignment);
        }

        static void free_aligned(void* ptr) {
            memoryManager.free(ptr);
        }

    private:
        static std::pair<std::string_view, std::string_view>
        parse_signature(std::string pattern_str) {
            size_t pos = 0;
            std::string mask_str;

            while ((pos = pattern_str.find(' ')) != std::string::npos) {
                pattern_str.replace(pos, 1, "");
            }

            mask_str.resize(pattern_str.size());
            for (size_t i = 0; i < mask_str.size(); ++i) {
                mask_str[i] = (pattern_str[i] == '?' ? '?' : 'x');
            }

            return { pattern_str, mask_str };
        }

    private:
        static size_t count_trailing_zeros(uint32_t value) {
#ifdef _MSC_VER
            //if (_MSC_VER >= 1920) {
            //    return __tzcnt_u32(value);
            //}
            //else {
            unsigned long index;
            _BitScanForward(&index, value);
            return index;
            //}
#else
            return __tzcnt_u32(value);
#endif
        }

    private:
        std::optional<ScanResult> scan_with_avx512(std::string pattern_str,
            const void* base_addr,
            size_t length) {
            auto [pattern, mask] = parse_signature(pattern_str);

            uint8_t* pattern_bytes = static_cast<uint8_t*>(
                allocate_aligned(64, 64));
            if (!pattern_bytes) return std::nullopt;

            memcpy(pattern_bytes, pattern.data(), pattern.length());

            __m512i pattern_vec = _mm512_loadu_si512(
                reinterpret_cast<const __m512i*>(pattern_bytes));

            const uint8_t* mem_ptr = static_cast<const uint8_t*>(base_addr);
            size_t offset = 0;

            while (length >= 64) {
                __m512i memory_chunk = _mm512_loadu_si512(mem_ptr);

                __mmask64 matches = _mm512_cmpeq_epu8_mask(
                    _mm512_and_si512(memory_chunk, pattern_vec),
                    pattern_vec
                );

                if (matches != 0) {
                    size_t match_pos = count_trailing_zeros(
                        static_cast<uint32_t>(matches));
                    free_aligned(pattern_bytes);
                    return ScanResult{
                        .signature_name = pattern.substr(match_pos, pattern.length()),
                        .address = reinterpret_cast<uintptr_t>(mem_ptr + match_pos),
                        .module_base = reinterpret_cast<uintptr_t>(base_addr),
                        .module_size = length
                    };
                }

                mem_ptr += 64;
                length -= 64;
                offset += 64;
            }

            free_aligned(pattern_bytes);
            return std::nullopt;
        }

    private:
        std::optional<ScanResult> scan_with_avx2(std::string pattern_str,
            const void* base_addr,
            size_t length) {
            if (pattern_str.empty() || !base_addr || length == 0) {
                return std::nullopt;
            }

            auto [pattern, mask] = parse_signature(pattern_str);
            if (pattern.empty()) {
                return std::nullopt;
            }

            uint8_t* pattern_bytes = static_cast<uint8_t*>(allocate_aligned(32, 32));
            if (!pattern_bytes) {
                return std::nullopt;
            }

            memcpy(pattern_bytes, pattern.data(), pattern.length());

            __m256i pattern_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pattern_bytes));

            const uint8_t* mem_ptr = static_cast<const uint8_t*>(base_addr);
            size_t offset = 0;

            while (length >= 32) {
                __m256i memory_chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(mem_ptr));
                __m256i matches = _mm256_cmpeq_epi8(
                    _mm256_and_si256(memory_chunk, pattern_vec),
                    pattern_vec
                );

                uint32_t match_mask = _mm256_movemask_epi8(matches);
                if (match_mask != 0) {
                    size_t match_pos = count_trailing_zeros(match_mask);

                    ScanResult sr = ScanResult{
                        .signature_name = pattern.substr(match_pos, pattern.length()),
                        .address = reinterpret_cast<uintptr_t>(mem_ptr + match_pos),
                        .module_base = reinterpret_cast<uintptr_t>(base_addr),
                        .module_size = length
                    };

                    free_aligned(pattern_bytes);
                    return sr;
                }

                mem_ptr += 32;
                length -= 32;
                offset += 32;
            }

            free_aligned(pattern_bytes);
            return std::nullopt;
        }

    private:
        std::optional<ScanResult> scan_with_sse(std::string pattern_str,
            const void* base_addr,
            size_t length) {
            if (pattern_str.empty() || !base_addr || length == 0) {
                return std::nullopt;
            }

            auto [pattern, mask] = parse_signature(pattern_str);
            if (pattern.empty() || mask.empty()) {
                return std::nullopt;
            }

            uint8_t* pattern_bytes = static_cast<uint8_t*>(allocate_aligned(16, 16));
            if (!pattern_bytes) {
                return std::nullopt;
            }

            size_t pattern_size = pattern.length();
            size_t copy_size = std::min<size_t>(pattern_size, 16u);
            memcpy(pattern_bytes, pattern.data(), copy_size);

            __m128i pattern_vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pattern_bytes));
            const uint8_t* mem_ptr = static_cast<const uint8_t*>(base_addr);
            size_t offset = 0;

            while (length >= 16) {
                if (reinterpret_cast<uintptr_t>(mem_ptr) + 16 >
                    reinterpret_cast<uintptr_t>(base_addr) + length) {
                    break;
                }

                __m128i memory_chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mem_ptr));
                __m128i matches = _mm_cmpeq_epi8(
                    _mm_and_si128(memory_chunk, pattern_vec),
                    pattern_vec
                );

                int match_mask = _mm_movemask_epi8(matches);
                if (match_mask != 0) {
                    size_t match_pos = count_trailing_zeros(
                        static_cast<uint32_t>(match_mask));

                    ScanResult result = {
                        .signature_name = pattern.substr(match_pos, pattern_size - match_pos),
                        .address = reinterpret_cast<uintptr_t>(mem_ptr + match_pos),
                        .module_base = reinterpret_cast<uintptr_t>(base_addr),
                        .module_size = length
                    };

                    free_aligned(pattern_bytes);
                    return result;
                }

                mem_ptr += 16;
                length -= 16;
                offset += 16;
            }

            free_aligned(pattern_bytes);
            return std::nullopt;
        }

    private:
        static auto scan_fallback(std::string signature,
            const void* base_addr,
            size_t length)
        {
            static auto pattern_to_byte = [](const char* pattern) {
                auto bytes = std::vector<std::optional<uint8_t>>{};
                auto start = const_cast<char*>(pattern);
                auto end = const_cast<char*>(pattern) + strlen(pattern);
                bytes.reserve(strlen(pattern) / 2);

                for (auto current = start; current < end; ++current) {
                    if (*current == '?') {
                        ++current;
                        if (*current == '?')
                            ++current;
                        bytes.push_back(std::nullopt);
                    }
                    else bytes.push_back((uint8_t)strtoul(current, &current, 16));
                }
                return bytes;
            };

            HMODULE gameModule = (HMODULE)base_addr;
            auto* const scanBytes = reinterpret_cast<uint8_t*>(gameModule);
            const auto pattern = pattern_to_byte(signature.c_str());
            const auto end = scanBytes + length;

            auto it = std::search(std::execution::par, scanBytes, end, pattern.cbegin(), pattern.cend(),
                [](auto byte, auto opt) {
                    return !opt.has_value() || *opt == byte;
                });

            auto ret = it != end ? (uintptr_t)it : 0u;
            return ScanResult{
                .signature_name = "fallback",
                .address = ret,
                .module_base = reinterpret_cast<uintptr_t>(base_addr),
                .module_size = length
            };
        }

    public:
        Scanner() {
            detected_is = detect_instruction_set();
        }

        std::optional<ScanResult> scan_pattern(std::string pattern_str,
            const void* base_addr,
            size_t length) {
            switch (detected_is) { // AVX2 & AVX512 can be buggy
                //case InstructionSet::AVX512:
                //    return scan_with_avx512(pattern_str, base_addr, length);// dont have an AVX512 CPU to test this, relying on hopes and dreams
                //case InstructionSet::AVX2:
                //    return scan_with_avx2(pattern_str, base_addr, length);// TODO: fix crashing with __aligned_free 
                case InstructionSet::SSE:
                    return scan_with_sse(pattern_str, base_addr, length);
                default:
                    return scan_fallback(pattern_str, base_addr, length);
            }
            return std::nullopt;
        }

        template <InstructionSet T>
        std::optional<ScanResult> custom_scan_pattern(std::string pattern_str,
            const void* base_addr,
            size_t length) {
            switch (T) {
                case InstructionSet::AVX512:
                    return scan_with_avx512(pattern_str, base_addr, length);
                case InstructionSet::AVX2:
                    return scan_with_avx2(pattern_str, base_addr, length);
                case InstructionSet::SSE:
                    return scan_with_sse(pattern_str, base_addr, length);
                default:
                    return scan_fallback(pattern_str, base_addr, length);
            }
            return std::nullopt;
        }

        std::string get_detected_cpu_features() const {
            std::vector<std::string> features;

            if (detected_is >= InstructionSet::AVX512) features.push_back("AVX512");
            if (detected_is >= InstructionSet::AVX2) features.push_back("AVX2");
            if (detected_is >= InstructionSet::SSE) features.push_back("SSE");

            std::string result;
            for (size_t i = 0; i < features.size(); ++i) {
                result += features[i];
                if (i < features.size() - 1) result += ", ";
            }

            return result.empty() ? "No SIMD support detected" : result;
        }

    private:
        InstructionSet detected_is;
    };

    static Scanner scanner;
};