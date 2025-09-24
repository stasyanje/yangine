#pragma once

#include "../common/GameTimer.h"
#include "../pch.h"

namespace canvas
{

enum class PSOType
{
    GRAPHICS,
    UI
};

struct DrawItem
{
    PSOType psoType{};

    D3D_PRIMITIVE_TOPOLOGY topology{D3D_PRIMITIVE_TOPOLOGY_UNDEFINED};
    UINT countPerInstance = 0;
    UINT instanceCount = 1;
    D3D12_VERTEX_BUFFER_VIEW vbv{};

    // Optional fields
    D3D12_INDEX_BUFFER_VIEW ibv{};
    D3D12_GPU_DESCRIPTOR_HANDLE srv{};
    D3D12_GPU_VIRTUAL_ADDRESS vsCB{};
    D3D12_GPU_VIRTUAL_ADDRESS psCB{};
};

} // namespace canvas