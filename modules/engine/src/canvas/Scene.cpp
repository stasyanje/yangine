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

std::vector<DrawItem> Scene::MakeDrawItems(const timer::Tick& tick)
{
    Update(tick);

    auto cbv = m_rendererServices.WritePerDrawCB(m_shaderConstants);
    auto meshViews = m_resourceFactory.GetMeshViews(m_meshHandle);

    DrawItem graphics{};
    graphics.psoType = PSOType::GRAPHICS;
    graphics.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    graphics.vsCB = cbv;
    graphics.vbv = meshViews.vbv;
    graphics.ibv = meshViews.ibv;
    graphics.countPerInstance = meshViews.countPerInstance;
    graphics.instanceCount = meshViews.instanceCount;

    // graphics.srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_srvDescriptorSize);

    auto uiMeshViews = m_resourceFactory.GetMeshViews(m_uiHandle);

    DrawItem ui{};
    ui.psoType = PSOType::UI;
    ui.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    ui.vbv = uiMeshViews.vbv;
    ui.countPerInstance = uiMeshViews.countPerInstance;
    ui.instanceCount = uiMeshViews.instanceCount;

    std::vector<DrawItem> drawItems;

    drawItems.push_back(graphics);
    drawItems.push_back(ui);

    return drawItems;
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