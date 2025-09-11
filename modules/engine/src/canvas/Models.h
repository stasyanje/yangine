#pragma once

#include <DirectXMath.h>

// MARK: - Primitives

typedef DirectX::XMINT2 Int2;
typedef DirectX::XMINT3 Int3;
typedef DirectX::XMFLOAT2 Float2;
typedef DirectX::XMFLOAT3 Float3;
typedef DirectX::XMFLOAT4 Float4;
typedef DirectX::XMFLOAT4X4 Float4x4;

inline bool IsZero(const Int2& value) noexcept
{
    return value.x == 0 && value.y == 0;
};
inline bool IsZero(const Int3& value) noexcept
{
    return value.x == 0 && value.y == 0 && value.z == 0;
};

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
    Float3 color;
};

// MARK: - CB

struct alignas(256) ShaderConstants
{
    Float4x4 model;
    Float4x4 viewProjection;
};

} // namespace canvas
