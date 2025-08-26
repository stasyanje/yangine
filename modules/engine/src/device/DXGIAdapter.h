#pragma once

#include "../pch.h"

namespace DX
{

class DXGIAdapter final
{
public:
    // Disallow copy / assign
    DXGIAdapter(const DXGIAdapter&) = delete;
    DXGIAdapter& operator=(const DXGIAdapter&) = delete;

    DXGIAdapter() noexcept = default;
    ~DXGIAdapter() noexcept = default;

    Microsoft::WRL::ComPtr<ID3D12Device> CreateDevice(IDXGIFactory6*);

    bool isDisplayHDR10(RECT windowBounds);

private:
    void Initialize(IDXGIFactory6*);
    void LogGPUMemoryInfo();
    void LogOutputs();
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

    D3D_FEATURE_LEVEL D3DFeatureLevel(ID3D12Device*);

    Microsoft::WRL::ComPtr<IDXGIAdapter3> m_dxgiAdapter;
};

} // namespace DX