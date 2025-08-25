#pragma once

#include "../pch.h"
#include "DXGIAdapter.h"

class DXGIFactory final
{
public:
    DXGIFactory() noexcept;
    ~DXGIFactory() noexcept = default;

    Microsoft::WRL::ComPtr<ID3D12Device> CreateDevice()
    {
        return m_dxgiAdapter->CreateDevice(m_dxgiFactory.Get());
    };

    DXGI_COLOR_SPACE_TYPE ColorSpace(
        HWND,
        DXGI_FORMAT backBufferFormat,
        bool isHDREnabled
    );

    Microsoft::WRL::ComPtr<IDXGISwapChain1> CreateSwapChain(
        HWND,
        ID3D12CommandQueue*,
        DXGI_SWAP_CHAIN_DESC1,
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC
    );

    bool IsCurrent();
    bool isTearingAllowed();
    void Reinitialize();

private:
    void InitializeDebugLayer();

    DWORD m_dxgiFactoryFlags = 0;
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;
    std::unique_ptr<DX::DXGIAdapter> m_dxgiAdapter = nullptr;
};