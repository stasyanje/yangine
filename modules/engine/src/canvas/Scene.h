#pragma once

#include "../pch.h"
#include "DrawItem.h"
#include "Models.h"
#include <memory>
#include <vector>

// Forward declarations
namespace input
{
class InputController;
}
namespace window
{
class WindowStateReducer;
}

namespace canvas
{
// Forward declarations
class Camera;

using MeshHandle = uint32_t;

struct SubmeshRange
{
    UINT indexCount{};
    UINT startIndex{};
    INT baseVertex{};
    D3D_PRIMITIVE_TOPOLOGY topology{D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST};
};

struct MeshViews
{
    D3D12_VERTEX_BUFFER_VIEW vbv{};
    D3D12_INDEX_BUFFER_VIEW ibv{};
    std::vector<SubmeshRange> parts;
};

enum class MeshDesc
{
    CUBES,
    UI
};

class ResourceFactory
{
public:
    virtual ~ResourceFactory() = default;
    virtual MeshHandle LoadMesh(const MeshDesc&) = 0;
    virtual void UnloadMesh(MeshHandle) = 0;
    virtual MeshViews GetMeshViews(MeshHandle) = 0;
};

class RendererServices
{
public:
    virtual ~RendererServices() = default;
    virtual D3D12_GPU_VIRTUAL_ADDRESS WritePerDrawCB(const ShaderConstants& data) = 0;
};

class Scene final
{
public:
    Scene(
        ResourceFactory& resourceFactory,
        RendererServices& rendererServices,
        input::InputController* inputController,
        window::WindowStateReducer* stateReducer
    ) noexcept;

    void OnEnter();
    void OnExit();

    void Update(const timer::Tick& tick);
    std::vector<DrawItem> MakeDrawItems();

private:
    ShaderConstants m_shaderConstants;
    MeshHandle m_meshHandle;
    MeshHandle m_uiHandle;

    std::unique_ptr<Camera> m_camera;

    ResourceFactory& m_resourceFactory;
    RendererServices& m_rendererServices;
};
} // namespace canvas