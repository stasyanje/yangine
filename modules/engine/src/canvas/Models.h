#pragma once

#include <DirectXMath.h>

// MARK: - Primitives

typedef DirectX::XMFLOAT2 Float2;
typedef DirectX::XMFLOAT3 Float3;
typedef DirectX::XMFLOAT4 Float4;
typedef DirectX::XMFLOAT4X4 Float4x4;

namespace canvas
{

// MARK: - Custom

struct RectF
{
    float left;
    float top;
    float right;
    float bottom;
};

struct Vertex
{
    Float3 position;
};

// MARK: - CB

struct alignas(256) ShaderConstants
{
    Float4x4 model;
    Float4x4 viewProjection;
};

} // namespace canvas
