#pragma once

#define NOMINMAX

#include <Windows.h>
#include <winternl.h>
#include <vector>
#include <string>
#include <optional>
#include <string_view>
#include <algorithm>
#include <Dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

#include "Memory.h"


namespace MemLib {
    struct SectionHeader {
        std::string name;
        DWORD virtual_size;
        DWORD virtual_address;
        DWORD size_of_raw_data;
        DWORD pointer_to_raw_data;
        DWORD characteristics;
    };

    struct ExportEntry {
        std::string name;
        DWORD ordinal;
        DWORD address;
    };

    struct ImportEntry {
        std::string dll_name;
        std::vector<std::string> functions;
    };

    struct SymbolSearchResult {
        bool found;
        void* base;
        PDWORD rva;
        std::string symbolName;
    };

    class Module {
    public:
        static SectionHeader get_module_information(const std::string& name) {
            HMODULE handle = GetModuleHandleA(name.c_str());
            DWORD base = MemLib::Module::get_module_base(name).value();
            DWORD size = MemLib::Module::get_module_size(name).value();

            return {
                name,
                base,
                size,
                base,
                size, 0
            };
        }

        static SymbolSearchResult find_function_by_export(HMODULE peBase, const std::string& symbolName) {
            SymbolSearchResult result = { false, nullptr, 0, "" };

            if (peBase == NULL)
                return result;

            PIMAGE_DOS_HEADER imageDosHeader = (PIMAGE_DOS_HEADER)peBase;

            if (imageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
                return result;

            PIMAGE_NT_HEADERS imageNtHeaders = (PIMAGE_NT_HEADERS)((unsigned char*)imageDosHeader + imageDosHeader->e_lfanew);

            if (imageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
                return result;

            PIMAGE_OPTIONAL_HEADER imageOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&imageNtHeaders->OptionalHeader;
            PIMAGE_DATA_DIRECTORY imageExportDataDirectory = &(imageOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
            PIMAGE_EXPORT_DIRECTORY imageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((unsigned char*)peBase + imageExportDataDirectory->VirtualAddress);

            DWORD numberOfNames = imageExportDirectory->NumberOfNames;

            PDWORD exportAddressTable = (PDWORD)((unsigned char*)peBase + imageExportDirectory->AddressOfFunctions);
            PWORD nameOrdinalsPointer = (PWORD)((unsigned char*)peBase + imageExportDirectory->AddressOfNameOrdinals);
            PDWORD exportNamePointerTable = (PDWORD)((unsigned char*)peBase + imageExportDirectory->AddressOfNames);

            char buffer[1024 * 20] = { 0 }; // may need to be global
            int nameIndex = 0;
            for (nameIndex = 0; nameIndex < numberOfNames; nameIndex++)
            {
                char* name = (char*)((unsigned char*)peBase + exportNamePointerTable[nameIndex]);
                if (strcmp(symbolName.c_str(), name) == 0)
                {
                    WORD ordinal = nameOrdinalsPointer[nameIndex];
                    PDWORD targetFunctionAddress = (PDWORD)((unsigned char*)peBase + exportAddressTable[ordinal]);

                    result.found = true;
                    result.rva = targetFunctionAddress;
                    result.symbolName = name;
                    result.base = reinterpret_cast<void*>(peBase);
                    return result;
                }
            }

            return result;
        }

        static std::optional<DWORD64> get_module_base(const std::string& module_name) {
            HMODULE hModule = GetModuleHandleA(module_name.c_str());
            if (!hModule) {
                return std::nullopt;
            }
            return reinterpret_cast<DWORD64>(hModule);
        }

        static std::optional<DWORD64> get_module_size(HMODULE hModule) {
            if (!hModule) return std::nullopt;

            DWORD size = 0;
            MEMORY_BASIC_INFORMATION mbi;

            for (DWORD64 addr = reinterpret_cast<DWORD64>(hModule);
                VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) == sizeof(mbi);
                addr += mbi.RegionSize) {
                if (mbi.AllocationBase == hModule &&
                    mbi.State == MEM_COMMIT &&
                    mbi.Protect != PAGE_NOACCESS) {
                    size += mbi.RegionSize;
                }
            }

            return size;
        }


        static std::optional<DWORD64> get_module_size(const std::string& module_name) {
            HMODULE hModule = GetModuleHandleA(module_name.c_str());
            if (!hModule) {
                return std::nullopt;
            }
            return get_module_size(hModule);
        }

        static std::vector<ExportEntry> parse_export_table(HMODULE hModule) {
            if (!hModule) return {};

            PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
            if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
                throw std::invalid_argument("Invalid DOS header");
            }

            DWORD64 ntOffset = dosHeader->e_lfanew;
            if (ntOffset == 0 || ntOffset > 0x1000) {
                throw std::invalid_argument("Invalid NT header offset");
            }

            PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
                reinterpret_cast<DWORD_PTR>(hModule) + ntOffset);

            if (!memoryManager.is_valid_memory(ntHeaders)) {
                throw std::runtime_error("Invalid NT headers memory");
            }

            if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
                throw std::invalid_argument("Invalid PE signature");
            }

            PIMAGE_EXPORT_DIRECTORY exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
                reinterpret_cast<DWORD_PTR>(hModule) +
                ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

            if (!memoryManager.is_valid_memory(exportDir)) {
                throw std::runtime_error("Invalid export directory memory");
            }

            std::vector<ExportEntry> exports;
            if (!exportDir) return exports;

            DWORD* names = reinterpret_cast<DWORD*>(
                reinterpret_cast<DWORD_PTR>(hModule) + exportDir->AddressOfNames);

            if (!memoryManager.is_valid_memory(names)) {
                throw std::runtime_error("Invalid export names memory");
            }

            for (DWORD i = 0; i < exportDir->NumberOfNames; ++i) {
                if (names[i] == 0) continue;

                ExportEntry entry;
                entry.name = reinterpret_cast<const char*>(
                    reinterpret_cast<DWORD_PTR>(hModule) + names[i]);
                entry.ordinal = *reinterpret_cast<DWORD*>(
                    reinterpret_cast<DWORD_PTR>(hModule) + exportDir->AddressOfNameOrdinals + i * sizeof(DWORD));
                entry.address = *reinterpret_cast<DWORD*>(
                    reinterpret_cast<DWORD_PTR>(hModule) + exportDir->AddressOfFunctions + i * sizeof(DWORD));

                exports.push_back(entry);
            }

            return exports;
        }

        static std::vector<ImportEntry> parse_import_table(HMODULE hModule) {
            std::vector<ImportEntry> imports;

            if (!hModule) return imports;

            PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
            if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
                return imports;
            }

            PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
                reinterpret_cast<DWORD_PTR>(hModule) + dosHeader->e_lfanew);

            if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
                return imports;
            }

            auto& importDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
            if (!importDir.VirtualAddress || !importDir.Size) {
                return imports;
            }

            DWORD moduleBase = reinterpret_cast<DWORD_PTR>(hModule);
            DWORD moduleSize = ntHeaders->OptionalHeader.SizeOfImage;
            DWORD importDirEnd = importDir.VirtualAddress + importDir.Size;

            if (importDirEnd > moduleSize) {
                return imports;
            }

            PIMAGE_IMPORT_DESCRIPTOR importDesc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
                moduleBase + importDir.VirtualAddress);

            while (importDesc && importDesc->Name) {
                ImportEntry entry;

                DWORD dllNameRVA = importDesc->Name;
                if (dllNameRVA >= moduleSize) break;

                const char* dllNamePtr = reinterpret_cast<const char*>(moduleBase + dllNameRVA);
                if (!dllNamePtr) break;

                entry.dll_name = dllNamePtr;

                DWORD otfRVA = importDesc->OriginalFirstThunk;
                if (otfRVA >= moduleSize) break;

                PIMAGE_THUNK_DATA32 originalFirstThunk = reinterpret_cast<PIMAGE_THUNK_DATA32>(
                    moduleBase + otfRVA);

                if (originalFirstThunk) {
                    DWORD currentRVA = reinterpret_cast<DWORD_PTR>(originalFirstThunk);

                    while (currentRVA &&
                        currentRVA < moduleBase + moduleSize &&
                        *reinterpret_cast<DWORD*>(currentRVA)) {
                        DWORD thunkValue = *reinterpret_cast<DWORD*>(currentRVA);

                        if (thunkValue & IMAGE_ORDINAL_FLAG64) {
                            entry.functions.push_back(
                                std::to_string(thunkValue & 0xFFFF));
                        }
                        else {
                            DWORD importByNameRVA = thunkValue;
                            if (importByNameRVA >= moduleSize) break;

                            PIMAGE_IMPORT_BY_NAME importByName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(
                                moduleBase + importByNameRVA);

                            if (importByName && importByName->Name) {
                                entry.functions.push_back(importByName->Name);
                            }
                        }

                        currentRVA += sizeof(DWORD);
                    }
                }

                imports.push_back(entry);
                ++importDesc;
            }

            return imports;
        }

        static std::vector<SectionHeader> analyze_sections(HMODULE hModule) {
            std::vector<SectionHeader> sections;

            if (!hModule) return sections;

            PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
            PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
                reinterpret_cast<DWORD_PTR>(hModule) + dosHeader->e_lfanew);

            PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);

            for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
                SectionHeader section;
                section.name = std::string(
                    reinterpret_cast<const char*>(sectionHeader->Name),
                    std::min<size_t>(sizeof(sectionHeader->Name), size_t(8)));
                section.virtual_size = sectionHeader->Misc.VirtualSize;
                section.virtual_address = sectionHeader->VirtualAddress;
                section.size_of_raw_data = sectionHeader->SizeOfRawData;
                section.pointer_to_raw_data = sectionHeader->PointerToRawData;
                section.characteristics = sectionHeader->Characteristics;

                sections.push_back(section);
                ++sectionHeader;
            }

            return sections;
        }
    };
};