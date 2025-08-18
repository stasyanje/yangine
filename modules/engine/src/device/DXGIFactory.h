#pragma once

#include "../pch.h"

class DXGIFactory final
{
public:
    DXGIFactory() noexcept;
    ~DXGIFactory() noexcept = default;

    bool IsCurrent();
    bool isTearingAllowed();
    void ClearCache();
    void LogGPUMemoryInfo(LUID adapterLuid);

    Microsoft::WRL::ComPtr<ID3D12Device> CreateDevice();

    DXGI_COLOR_SPACE_TYPE ColorSpace(
        HWND,
        DXGI_FORMAT backBufferFormat,
        bool isHDREnabled
    );

    Microsoft::WRL::ComPtr<IDXGISwapChain1> CreateSwapChain(
        HWND,
        ID3D12CommandQueue*,
        DXGI_SWAP_CHAIN_DESC1
    );

private:
    DWORD m_dxgiFactoryFlags = 0;
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;

    Microsoft::WRL::ComPtr<IDXGIAdapter1> GetAdapter(D3D_FEATURE_LEVEL);
    bool isDisplayHDR10(RECT windowBounds);

    D3D_FEATURE_LEVEL D3DFeatureLevel(ID3D12Device*);
};