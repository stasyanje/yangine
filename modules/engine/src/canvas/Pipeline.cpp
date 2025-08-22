//
// Game.cpp
//

#include "Pipeline.h"
#include "../common/AsyncLogger.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"
#include "../pch.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace canvas;

using Microsoft::WRL::ComPtr;

Pipeline::Pipeline(
    input::InputController* inputController,
    DX::DeviceResources* deviceResources,
    window::WindowStateReducer* stateReducer
) :
    m_inputController(inputController),
    m_deviceResources(deviceResources),
    m_stateReducer(stateReducer)
{
}

void Pipeline::Initialize()
{
    auto device = m_deviceResources->GetD3DDevice();

    CreateVertexBuffer(device);
    CreateSignature(device);
    CreatePSO(device);
}

void Pipeline::Deinitialize()
{
    m_rootSignature.Reset();
    m_pipelineState.Reset();
    m_vertexBuffer.Reset();
}

void Pipeline::Prepare(ID3D12GraphicsCommandList* commandList)
{
    UpdateTrianglePosition();

    // Set pipeline state
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    // Set primitive topology
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set vertex buffer
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
}

// Draws the scene.
void Pipeline::Draw(ID3D12GraphicsCommandList* commandList)
{
    // Draw
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");
    commandList->DrawInstanced(3, 1, 0, 0);
    PIXEndEvent(commandList);
}

void Pipeline::CreateVertexBuffer(ID3D12Device* device)
{
    // Define triangle vertices
    struct Vertex
    {
        XMFLOAT3 position{};
        XMFLOAT4 color{};
    };

    Vertex triangleVertices[] = {{}, {}, {}};
    const UINT vertexBufferSize = sizeof(triangleVertices);

    // Create vertex buffer
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

    DX::ThrowIfFailed(
        device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &vertexBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_vertexBuffer.GetAddressOf())
        ),
        "CreateTriangleResources: CreateCommittedResource"
    );

    // Copy vertex data to the vertex buffer
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(
        m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)),
        "CreateTriangleResources: m_vertexBuffer->Map"
    );
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
}

void Pipeline::CreatePSO(ID3D12Device* device)
{
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    DX::ThrowIfFailed(
        D3DCompileFromFile(L"assets\\engine\\shaders\\TriangleVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr)
    );
    DX::ThrowIfFailed(
        D3DCompileFromFile(L"assets\\engine\\shaders\\TrianglePixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr)
    );

    // Define the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };

    // Create graphics pipeline state
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
    psoDesc.SampleDesc.Count = 1;

    DX::ThrowIfFailed(
        device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)),
        "CreateTriangleResources: CreateGraphicsPipelineState"
    );
}

void Pipeline::CreateSignature(ID3D12Device* device)
{
    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 /*t0*/, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    CD3DX12_ROOT_PARAMETER1 params[2];
    params[0].InitAsConstantBufferView(0 /*b0*/, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC);
    params[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rsDesc;
    rsDesc.Init_1_1(_countof(params), params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> sig, err;
    D3DX12SerializeVersionedRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &sig, &err);

    device->CreateRootSignature(
        0,
        sig->GetBufferPointer(),
        sig->GetBufferSize(),
        IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf())
    );
}

void Pipeline::UpdateTrianglePosition()
{
    if (!m_inputController || !m_vertexBuffer)
        return;

    // Get mouse position from input controller
    auto mousePos = m_inputController->MousePosition();

    // Get window size to convert pixels to normalized device coordinates
    auto outputSize = m_stateReducer->getBounds();
    float windowWidth = static_cast<float>(outputSize.right - outputSize.left);
    float windowHeight = static_cast<float>(outputSize.bottom - outputSize.top);

    // Convert mouse position from pixels to NDC (-1 to 1 range)
    float centerX = (mousePos.x / windowWidth) * 2.0f - 1.0f;
    float centerY = -((mousePos.y / windowHeight) * 2.0f - 1.0f); // Flip Y axis

    // Define triangle vertices relative to mouse position
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };

    Vertex triangleVertices[] =
        {
            {XMFLOAT3(centerX, centerY + 0.05f, 0.0f),
             XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f)},
            {XMFLOAT3(centerX + 0.0375f, centerY - 0.05f, 0.0f),
             XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f)},
            {XMFLOAT3(centerX - 0.0375f, centerY - 0.05f, 0.0f),
             XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f)}
        };

    // Update vertex buffer with new positions
    void* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);

    DX::ThrowIfFailed(
        m_vertexBuffer->Map(0, &readRange, &pVertexDataBegin),
        "UpdateTrianglePosition: m_vertexBuffer->Map"
    );

    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    m_vertexBuffer->Unmap(0, nullptr);
}