#pragma once

#include "../pch.h"

namespace DX
{
struct BufferParams
{
    static constexpr size_t MAX_BACK_BUFFER_COUNT = 3;

    const UINT count = 2;
    const DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;
    const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
};

inline DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt) noexcept
{
    switch (fmt) {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8X8_UNORM;
    default:
        return fmt;
    }
}
} // namespace DX