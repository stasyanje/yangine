#pragma once

#include "../pch.h"

#include <DirectXMath.h>
#include <array>
#include <cmath>

using namespace DirectX;

namespace canvas
{

struct VertexPosColor
{
    XMFLOAT3 position;
    XMFLOAT4 color;
};

inline std::array<VertexPosColor, 3>
MakeTriangle(
    float cx,
    float cy,
    const XMFLOAT4& cTop = XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
    const XMFLOAT4& cRight = XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f),
    const XMFLOAT4& cLeft = XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f)
)
{
    static constexpr float kHalfWidth = 0.0375f;
    static constexpr float kHalfHeight = 0.05f;

    return {
        VertexPosColor{XMFLOAT3(cx, cy + kHalfHeight, 0.0f), cTop},
        VertexPosColor{XMFLOAT3(cx + kHalfWidth, cy - kHalfHeight, 0.0f), cRight},
        VertexPosColor{XMFLOAT3(cx - kHalfWidth, cy - kHalfHeight, 0.0f), cLeft}
    };
}

inline std::array<VertexPosColor, 6>
Pack(const std::array<VertexPosColor, 3>& a, const std::array<VertexPosColor, 3>& b)
{
    return {a[0], a[1], a[2], b[0], b[1], b[2]};
}

} // namespace canvas