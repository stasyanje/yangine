#include "Scene.h"
#include "../input/InputController.h"
#include "../window/WindowStateReducer.h"
#include "Camera.h"

using namespace DirectX;
using namespace canvas;

Scene::Scene(
    ResourceFactory& resourceFactory,
    RendererServices& rendererServices,
    input::InputController* inputController,
    window::WindowStateReducer* stateReducer
) noexcept :
    m_camera(std::make_unique<Camera>(inputController, stateReducer)),
    m_resourceFactory(resourceFactory),
    m_rendererServices(rendererServices)
{
}

void Scene::OnEnter()
{
    m_meshHandle = m_resourceFactory.LoadMesh(MeshDesc::CUBES);
    m_uiHandle = m_resourceFactory.LoadMesh(MeshDesc::UI);
}

void Scene::OnExit()
{
    m_resourceFactory.UnloadMesh(m_meshHandle);
    m_resourceFactory.UnloadMesh(m_uiHandle);
}

void Scene::Update(const timer::Tick& tick)
{
    m_camera->Prepare(tick);

    XMStoreFloat4x4(
        &m_shaderConstants.viewProjection,
        XMMatrixTranspose(m_camera->CameraViewProjection())
    );

    XMMATRIX M = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    XMStoreFloat4x4(&m_shaderConstants.model, XMMatrixTranspose(M));

    double pitch = XM_2PI * std::fmod(tick.totalTime, 1.0);

    XMStoreFloat4x4(
        &m_shaderConstants.modelRotated,
        XMMatrixTranspose(M * XMMatrixRotationRollPitchYaw(0.0, pitch, 0.0))
    );
    m_shaderConstants.time = static_cast<float>(tick.totalTime);
}

inline DrawItem BaseDrawItem(MeshViews meshViews, SubmeshRange submesh)
{
    DrawItem di{};

    di.vbv = meshViews.vbv;
    di.ibv = meshViews.ibv;
    di.topology = submesh.topology;
    di.countPerInstance = submesh.indexCount;

    return di;
}

std::vector<DrawItem> Scene::MakeDrawItems()
{
    std::vector<DrawItem> drawItems;

    auto graphics = m_resourceFactory.GetMeshViews(m_meshHandle);

    for (auto submesh : graphics.parts) {
        DrawItem di = BaseDrawItem(graphics, submesh);
        di.vsCB = m_rendererServices.WritePerDrawCB(m_shaderConstants);
        di.psoType = PSOType::GRAPHICS;
        di.instanceCount = 7;

        drawItems.push_back(di);
    }

    // graphics.srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_srvDescriptorSize);

    auto ui = m_resourceFactory.GetMeshViews(m_uiHandle);

    for (auto submesh : ui.parts) {
        DrawItem di = BaseDrawItem(ui, submesh);
        di.psoType = PSOType::UI;

        drawItems.push_back(di);
    }

    return drawItems;
}