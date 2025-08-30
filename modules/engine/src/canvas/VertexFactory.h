#pragma once

#include "../pch.h"
#include "Models.h"
#include <array>

namespace canvas
{

inline std::array<Vertex, 8> MakeCubeVertices()
{
    return {
        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f)}, // 0: left-bottom-back
        Vertex{DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f)},  // 1: right-bottom-back
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f)},   // 2: right-top-back
        Vertex{DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f)},  // 3: left-top-back
        Vertex{DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f)},  // 4: left-bottom-front
        Vertex{DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f)},   // 5: right-bottom-front
        Vertex{DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)},    // 6: right-top-front
        Vertex{DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f)},   // 7: left-top-front
    };
}

inline std::array<uint16_t, 36> MakeCubeIndices()
{
    return {
        // Front face (z = +1)
        4, 7, 6, 4, 6, 5,
        // Back face (z = -1) 
        0, 2, 3, 0, 1, 2,
        // Left face (x = -1)
        0, 3, 7, 0, 7, 4,
        // Right face (x = +1)
        1, 6, 2, 1, 5, 6,
        // Top face (y = +1)
        3, 2, 6, 3, 6, 7,
        // Bottom face (y = -1)
        0, 4, 5, 0, 5, 1,
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