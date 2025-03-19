#pragma once

#include <ImGui/imgui.h>

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


namespace DXExternals {
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	WNDPROC	oWndProc = nullptr;

	typedef long(__fastcall* ResizeBuffersD3D12)(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	ResizeBuffersD3D12 oResizeBuffersD3D12;

	typedef ULONG(__fastcall* ReleaseD3D12)(IDXGISwapChain3* pSwapChain);
	ReleaseD3D12 oReleaseD3D12;

	typedef long(__fastcall* PresentD3D12) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	PresentD3D12 oPresentD3D12;

	typedef void(__fastcall* DrawInstancedD3D12)(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
	DrawInstancedD3D12 oDrawInstancedD3D12;

	typedef void(__fastcall* DrawIndexedInstancedD3D12)(ID3D12GraphicsCommandList* dCommandList, UINT IndexCount, UINT InstanceCount, UINT StartIndex, INT BaseVertex);
	DrawIndexedInstancedD3D12 oDrawIndexedInstancedD3D12;

	void(*oExecuteCommandListsD3D12)(ID3D12CommandQueue*, UINT, ID3D12CommandList*);
	HRESULT(*oSignalD3D12)(ID3D12CommandQueue*, ID3D12Fence*, UINT64);
}