#pragma once

#include <MCBase/Hooks/Hook.h>

#include <wrl/client.h>
#include <d2d1.h>
#include <d2d1_2.h>
#include <dwrite.h>
#include <d3d11on12.h>
#include <dxgi1_4.h>
#include <d2d1_3.h>
#include <dxgi.h>
#include <d3d12.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

class DirectXHook : public Hook {
public:
	DirectXHook();
	~DirectXHook();

	void Enable();
	void Disable();

	static void CopyBitmap(ID2D1DeviceContext* ctx, Microsoft::WRL::ComPtr<ID2D1Bitmap1>& from, Microsoft::WRL::ComPtr<ID2D1Bitmap>& to);
	static void CreateD11o12DeviceContext();
	static void ClearFrameContext(size_t index);
	static void ReleaseRender();

	static void RenderBlur(float strength);
	static void RenderDirectionalBlur(float strength, float angle);

	static long __fastcall HookResizeBuffersD3D12(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	static ULONG __fastcall HookReleaseD3D12(IDXGISwapChain3* pSwapChain);
	static long __fastcall hookPresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
	static void __fastcall hookDrawInstancedD3D12(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
	static void __fastcall hookDrawIndexedInstancedD3D12(ID3D12GraphicsCommandList* dCommandList, UINT IndexCount, UINT InstanceCount, UINT StartIndex, INT BaseVertex);
	static void hookExecuteCommandListsD3D12(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists);
	static HRESULT hookSignalD3D12(ID3D12CommandQueue* queue, ID3D12Fence* fence, UINT64 value);
};