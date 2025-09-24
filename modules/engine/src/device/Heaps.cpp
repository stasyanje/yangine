#include "Heaps.h"

using namespace DX;

Heaps::Heaps(ID3D12Device* device) :
    m_device(device),
    m_resourceFactory(std::make_unique<device::ResourceFactory>(device))
{
}

void Heaps::Initialize(UINT width, UINT height, bool reverseDepth)
{
    CreateDescriptorHeaps();
    InitializeDSV(width, height, reverseDepth);

    // Reset render targets in case of repeated initialization
    for (UINT n = 0; n < m_bufferParams.count; n++) {
        m_renderTargets[n].Reset();
    }
}

void Heaps::CreateDescriptorHeaps()
{
    // Create descriptor heaps for render target views and depth stencil views.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = m_bufferParams.count;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    ThrowIfFailed(m_device->CreateDescriptorHeap(
        &rtvDescriptorHeapDesc,
        IID_PPV_ARGS(m_rtvDescriptorHeap.ReleaseAndGetAddressOf())
    ));
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create depth stencil view
    if (m_bufferParams.depthBufferFormat != DXGI_FORMAT_UNKNOWN) {
        D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
        dsvDescriptorHeapDesc.NumDescriptors = 1;
        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        ThrowIfFailed(m_device->CreateDescriptorHeap(
            &dsvDescriptorHeapDesc,
            IID_PPV_ARGS(m_dsvDescriptorHeap.ReleaseAndGetAddressOf())
        ));
    }
}

void Heaps::InitializeDSV(UINT width, UINT height, bool reverseDepth)
{
    if (m_bufferParams.depthBufferFormat != DXGI_FORMAT_UNKNOWN) {
        D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            m_bufferParams.depthBufferFormat,
            width,
            height,
            1, // Use a single array entry.
            1  // Use a single mipmap level.
        );
        depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        // Allocate a 2-D surface as the depth/stencil buffer and create a depth/stencil view
        // on this surface.
        m_depthBuffer = m_resourceFactory->CreateDepthStencilTexture(
            width,
            height,
            m_bufferParams.depthBufferFormat
        );

        m_depthBuffer->SetName(L"Depth stencil");

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = m_bufferParams.depthBufferFormat;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        const auto cpuHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        m_device->CreateDepthStencilView(m_depthBuffer.Get(), &dsvDesc, cpuHandle);
    }
}

void Heaps::CreateRTargets(IDXGISwapChain* swapChain)
{
    // Obtain the back buffers for this window which will be the final render targets
    // and create render target views for each of them.
    for (UINT n = 0; n < m_bufferParams.count; n++) {
        ThrowIfFailed(swapChain->GetBuffer(n, IID_PPV_ARGS(m_renderTargets[n].GetAddressOf())));

        wchar_t name[25] = {};
        swprintf_s(name, L"Render target %u", n);
        m_renderTargets[n]->SetName(name);

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = m_bufferParams.format;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        m_device->CreateRenderTargetView(
            m_renderTargets[n].Get(),
            &rtvDesc,
            RTVHandle(n)
        );
    }
}

void Heaps::Prepare(ID3D12GraphicsCommandList* commandList, UINT backBufferIndex)
{
    const auto rtvDescriptor = RTVHandle(backBufferIndex);

    if (m_dsvDescriptorHeap == nullptr) {
        commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, nullptr);
        commandList->ClearRenderTargetView(rtvDescriptor, DirectX::Colors::CornflowerBlue, 0, nullptr);
        return;
    }

    auto dsvDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
    );

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, DirectX::Colors::CornflowerBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}