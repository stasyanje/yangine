//
// Game.h
//

#pragma once

#include "../common/GameTimer.h"
#include "../device/DeviceResources.h"
#include "../input/InputController.h"

namespace canvas
{
// A basic renderer implementation that creates a D3D12 device and
// provides rendering functionality.
class ResourceHolder final
{
public:
    // Disallow copy / assign
    ResourceHolder(const ResourceHolder&) = delete;
    ResourceHolder& operator=(const ResourceHolder&) = delete;

    ResourceHolder(input::InputController*) noexcept;
    ~ResourceHolder() noexcept = default;

    void Initialize(ID3D12Device*);
    void Deinitialize();

    void Prepare(ID3D12GraphicsCommandList*, double deltaTime);

private:
    void CreateVertexBuffer(ID3D12Device*);
    void UpdateTrianglePosition(double totalTime);

    input::InputController* m_inputController;

    // Resources
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};
} // namespace canvas
