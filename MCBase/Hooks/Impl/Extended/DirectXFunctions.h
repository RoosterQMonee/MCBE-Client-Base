#pragma once

#include <Windows.h>
#include <cstdint>
#include <mutex>

#include "DirectXExternals.h"
#include <MemLib/MinHook/Minhook.h>


struct DXFrameContext {
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12Resource* main_render_target_resource = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE main_render_target_descriptor;

    Microsoft::WRL::ComPtr<IDXGISurface1> surface = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Resource> resource11 = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dRenderTarget = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> backBuffer = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> blur = nullptr;
};

class DXFunctions {
private:
    static inline uint64_t* m_table = nullptr;

    // Helper structure for cleanup resources
    struct CleanupResources {
        IDXGIAdapter* adapter = nullptr;
        ID3D12Device* device = nullptr;
        ID3D12CommandQueue* commandQueue = nullptr;
        ID3D12CommandAllocator* commandAllocator = nullptr;
        ID3D12GraphicsCommandList* commandList = nullptr;
        IDXGISwapChain* swapChain = nullptr;

        void cleanup() {
            if (swapChain) swapChain->Release();
            if (commandList) commandList->Release();
            if (commandAllocator) commandAllocator->Release();
            if (commandQueue) commandQueue->Release();
            if (device) device->Release();
            if (adapter) adapter->Release();
        }

        ~CleanupResources() { cleanup(); }
    };

public:
    static bool Init() noexcept {
        // Window creation
        WNDCLASSEX windowClass = []() {
            WNDCLASSEX wc = {};
            wc.cbSize = sizeof(WNDCLASSEX);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc = DefWindowProc;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.lpszClassName = L"DXWin";
            return wc;
        }();

        HWND window = CreateWindowEx(
            0, windowClass.lpszClassName, L"DXWin",
            WS_OVERLAPPEDWINDOW, 0, 0, 100, 100,
            nullptr, nullptr, windowClass.hInstance, nullptr
        );

        if (!window) {
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        // Load required DLLs
        auto libDXGI = GetModuleHandle(L"dxgi.dll");
        auto libD3D12 = GetModuleHandle(L"d3d12.dll");

        if (!libDXGI || !libD3D12) {
            DestroyWindow(window);
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        CleanupResources resources;

        // Initialize DXGI factory
        auto CreateDXGIFactory = reinterpret_cast<void*>(GetProcAddress(libDXGI, "CreateDXGIFactory"));
        if (!CreateDXGIFactory) {
            DestroyWindow(window);
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        IDXGIFactory* factory;
        if (FAILED(reinterpret_cast<HRESULT(__stdcall*)(const IID&, void**)>(CreateDXGIFactory)(
            __uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory)))) {
            DestroyWindow(window);
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        // Get adapter
        if (FAILED(factory->EnumAdapters(0, &resources.adapter))) {
            factory->Release();
            DestroyWindow(window);
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        // Create device
        auto D3D12CreateDevice = reinterpret_cast<void*>(GetProcAddress(libD3D12, "D3D12CreateDevice"));
        if (!D3D12CreateDevice || FAILED(reinterpret_cast<HRESULT(__stdcall*)(IUnknown*,
            D3D_FEATURE_LEVEL, const IID&, void**)>(D3D12CreateDevice)(
                resources.adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device),
                reinterpret_cast<void**>(&resources.device)))) {
            factory->Release();
            DestroyWindow(window);
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        // Create command queue
        D3D12_COMMAND_QUEUE_DESC queueDesc = {
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            0,
            D3D12_COMMAND_QUEUE_FLAG_NONE,
            0
        };

        if (FAILED(resources.device->CreateCommandQueue(&queueDesc,
            IID_PPV_ARGS(&resources.commandQueue)))) {
            factory->Release();
            DestroyWindow(window);
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        // Create command allocator
        if (FAILED(resources.device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&resources.commandAllocator)))) {
            factory->Release();
            DestroyWindow(window);
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        // Create command list
        if (FAILED(resources.device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            resources.commandAllocator, nullptr,
            IID_PPV_ARGS(&resources.commandList)))) {
            factory->Release();
            DestroyWindow(window);
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        // Setup swap chain
        DXGI_RATIONAL refreshRate = { 60, 1 };

        // Initialize MODE_DESC members directly
        DXGI_MODE_DESC bufferDesc = {
            100,
            100,
            refreshRate,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
            DXGI_MODE_SCALING_UNSPECIFIED
        };

        // Initialize SAMPLE_DESC members directly
        DXGI_SAMPLE_DESC sampleDesc = {
            1,
            0
        };

        // Initialize SWAP_CHAIN_DESC members directly
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {
            bufferDesc,
            sampleDesc,
            DXGI_USAGE_RENDER_TARGET_OUTPUT,
            2,
            window,
            TRUE,
            DXGI_SWAP_EFFECT_FLIP_DISCARD,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
        };

        if (FAILED(factory->CreateSwapChain(
            resources.commandQueue, &swapChainDesc,
            reinterpret_cast<IDXGISwapChain**>(&resources.swapChain)))) {
            factory->Release();
            DestroyWindow(window);
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        // Allocate memory for function table
        m_table = static_cast<uint64_t*>(calloc(150, sizeof(uint64_t)));
        if (!m_table) {
            factory->Release();
            DestroyWindow(window);
            UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
            return false;
        }

        // Copy function pointers
        memcpy(m_table, *(uint64_t**)resources.device, 44 * sizeof(uint64_t));
        memcpy(m_table + 44, *(uint64_t**)resources.commandQueue, 19 * sizeof(uint64_t));
        memcpy(m_table + 63, *(uint64_t**)resources.commandAllocator, 9 * sizeof(uint64_t));
        memcpy(m_table + 72, *(uint64_t**)resources.commandList, 60 * sizeof(uint64_t));
        memcpy(m_table + 132, *(uint64_t**)resources.swapChain, 18 * sizeof(uint64_t));

        MH_Initialize();

        // Cleanup remaining resources
        resources.cleanup();
        DestroyWindow(window);
        UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

        return true;
    }

    static void Shutdown()
    {
        MH_DisableHook(MH_ALL_HOOKS);

        ::free(m_table);
        m_table = NULL;
    }

    static bool Bind(uint16_t _index, void** _original, void* _function)
    {
        void* target = (void*)m_table[_index];
        if (MH_CreateHook(target, _function, _original) != MH_OK || MH_EnableHook(target) != MH_OK)
            return false;

        return true;
    }

    static void Unbind(uint16_t _index)
    {
        MH_DisableHook((void*)m_table[_index]);
    }
};