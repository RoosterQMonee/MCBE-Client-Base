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
		WNDCLASSEX windowClass;
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = DefWindowProc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = GetModuleHandle(NULL);
		windowClass.hIcon = NULL;
		windowClass.hCursor = NULL;
		windowClass.hbrBackground = NULL;
		windowClass.lpszMenuName = NULL;
		windowClass.lpszClassName = L"Kpaste";
		windowClass.hIconSm = NULL;

		::RegisterClassEx(&windowClass);

		HWND window = ::CreateWindow(windowClass.lpszClassName, L"KPDxWin", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

		HMODULE libDXGI;
		HMODULE libD3D12;
		if ((libDXGI = ::GetModuleHandle(L"dxgi.dll")) == NULL || (libD3D12 = ::GetModuleHandle(L"d3d12.dll")) == NULL)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return false;
		}

		void* CreateDXGIFactory;
		if ((CreateDXGIFactory = ::GetProcAddress(libDXGI, "CreateDXGIFactory")) == NULL)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return false;
		}

		IDXGIFactory* factory;
		if (((long(__stdcall*)(const IID&, void**))(CreateDXGIFactory))(__uuidof(IDXGIFactory), (void**)&factory) < 0)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return false;
		}

		IDXGIAdapter* adapter;
		if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return false;
		}

		void* D3D12CreateDevice;
		if ((D3D12CreateDevice = ::GetProcAddress(libD3D12, "D3D12CreateDevice")) == NULL)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return false;
		}

		ID3D12Device* device;
		if (((long(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**))(D3D12CreateDevice))(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&device) < 0)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return false;
		}

		D3D12_COMMAND_QUEUE_DESC queueDesc;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Priority = 0;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.NodeMask = 0;

		ID3D12CommandQueue* commandQueue;
		if (device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (void**)&commandQueue) < 0)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return false;
		}

		ID3D12CommandAllocator* commandAllocator;
		if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&commandAllocator) < 0)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return false;
		}

		ID3D12GraphicsCommandList* commandList;
		if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&commandList) < 0)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return false;
		}

		DXGI_RATIONAL refreshRate;
		refreshRate.Numerator = 60;
		refreshRate.Denominator = 1;

		DXGI_MODE_DESC bufferDesc;
		bufferDesc.Width = 100;
		bufferDesc.Height = 100;
		bufferDesc.RefreshRate = refreshRate;
		bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SAMPLE_DESC sampleDesc;
		sampleDesc.Count = 1;
		sampleDesc.Quality = 0;

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc = bufferDesc;
		swapChainDesc.SampleDesc = sampleDesc;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.OutputWindow = window;
		swapChainDesc.Windowed = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGISwapChain* swapChain;
		if (factory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChain) < 0)
		{
			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
			return false;
		}

		m_table = (uint64_t*)::calloc(150, sizeof(uint64_t));
		::memcpy(m_table, *(uint64_t**)device, 44 * sizeof(uint64_t));
		::memcpy(m_table + 44, *(uint64_t**)commandQueue, 19 * sizeof(uint64_t));
		::memcpy(m_table + 44 + 19, *(uint64_t**)commandAllocator, 9 * sizeof(uint64_t));
		::memcpy(m_table + 44 + 19 + 9, *(uint64_t**)commandList, 60 * sizeof(uint64_t));
		::memcpy(m_table + 44 + 19 + 9 + 60, *(uint64_t**)swapChain, 18 * sizeof(uint64_t));

		MH_Initialize();

		device->Release();
		device = NULL;

		commandQueue->Release();
		commandQueue = NULL;

		commandAllocator->Release();
		commandAllocator = NULL;

		commandList->Release();
		commandList = NULL;

		swapChain->Release();
		swapChain = NULL;

		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

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
		void* target = (void*)m_table[_index];
        MH_DisableHook(target);
    }
};