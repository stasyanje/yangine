#pragma once

#include "../pch.h"

class DXGIFactory final
{
public:
    DXGIFactory() noexcept;
    ~DXGIFactory();

    bool IsCurrent();
    bool isTearingAllowed();
    void ClearCache();
    void LogGPUMemoryInfo(LUID adapterLuid);

    void CreateDevice(ID3D12Device** ppDevice);

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
    DWORD m_dxgiFactoryFlags;
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;

    void GetAdapter(IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL);
    bool isDisplayHDR10(RECT windowBounds);

    D3D_FEATURE_LEVEL D3DFeatureLevel(ID3D12Device*);
};