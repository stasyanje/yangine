#pragma once

#include "../pch.h"

#include <DirectXMath.h>
#include <array>
#include <cmath>

namespace canvas
{

typedef DirectX::XMFLOAT3 Float3;
typedef DirectX::XMFLOAT4 Float4;

struct VertexPosColor
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

inline std::array<VertexPosColor, 3>
MakeTriangle(
    float cx,
    float cy,
    const DirectX::XMFLOAT4& cTop = Float4(1.0f, 0.5f, 0.0f, 1.0f),
    const DirectX::XMFLOAT4& cRight = Float4(0.0f, 0.5f, 1.0f, 1.0f),
    const DirectX::XMFLOAT4& cLeft = Float4(0.5f, 1.0f, 0.0f, 1.0f)
)
{
    static constexpr float kHalfWidth = 0.0375f;
    static constexpr float kHalfHeight = 0.05f;

    return {
        VertexPosColor{Float3(cx, cy + kHalfHeight, 0.0f), cTop},
        VertexPosColor{Float3(cx + kHalfWidth, cy - kHalfHeight, 0.0f), cRight},
        VertexPosColor{Float3(cx - kHalfWidth, cy - kHalfHeight, 0.0f), cLeft}
    };
}

inline std::array<VertexPosColor, 6>
Pack(const std::array<VertexPosColor, 3>& a, const std::array<VertexPosColor, 3>& b)
{
    return {a[0], a[1], a[2], b[0], b[1], b[2]};
}

} // namespace canvas