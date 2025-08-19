#pragma once

#include "../pch.h"

namespace DX
{

class DXGIAdapter final
{
public:
    DXGIAdapter() noexcept = default;
    ~DXGIAdapter() noexcept = default;

    Microsoft::WRL::ComPtr<ID3D12Device> CreateDevice(IDXGIFactory6*);

    bool isDisplayHDR10(RECT windowBounds);

private:
    Microsoft::WRL::ComPtr<IDXGIAdapter3> m_dxgiAdapter;

    void Initialize(IDXGIFactory6*);
    void LogGPUMemoryInfo();

    D3D_FEATURE_LEVEL D3DFeatureLevel(ID3D12Device*);
};

} // namespace DX