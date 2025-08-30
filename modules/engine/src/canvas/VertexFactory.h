#pragma once

#include "../pch.h"
#include "Models.h"
#include <array>

namespace canvas
{

inline std::array<Vertex, 36> MakeCube()
{
    return {
        // Front face (z = +1)
        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f)},
        Vertex{DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)},

        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f)},

        // Back face (z = -1)
        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f)},

        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f)},

        // Left face (x = -1)
        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f)},
        Vertex{DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f)},

        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f)},
        Vertex{DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f)},

        // Right face (x = +1)
        Vertex{DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)},

        Vertex{DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f)},

        // Top face (y = +1)
        Vertex{DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)},

        Vertex{DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f)},

        // Bottom face (y = -1)
        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f)},
        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f)},

        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f)},
        Vertex{DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f)},
    };
}

inline std::array<Vertex, 3> MakeTriangle(float cx = 0.0f, float cy = 0.0f)
{
    static constexpr float kHalfWidth = 0.0375f;
    static constexpr float kHalfHeight = 0.05f;

    return {
        Vertex{Float3(cx, cy + kHalfHeight, 0.0f)},
        Vertex{Float3(cx + kHalfWidth, cy - kHalfHeight, 0.0f)},
        Vertex{Float3(cx - kHalfWidth, cy - kHalfHeight, 0.0f)}
    };
}

} // namespace canvas