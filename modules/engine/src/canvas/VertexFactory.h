#pragma once

#include "../pch.h"
#include "Models.h"
#include <array>

namespace canvas
{

inline std::array<Vertex, 8> MakeCubeVertices()
{
    return {
        Vertex{{-1.0f, -1.0f, -1.0f}, {6.0f/255.0f,  34.0f/255.0f, 178.0f/255.0f}},
        Vertex{{1.0f, -1.0f, -1.0f}, {175.0f/255.0f, 102.0f/255.0f, 231.0f/255.0f}},
        Vertex{{1.0f, 1.0f, -1.0f}, {123.0f/255.0f,  37.0f/255.0f, 239.0f/255.0f}},
        Vertex{{-1.0f, 1.0f, -1.0f}, {255.0f/255.0f,  41.0f/255.0f, 194.0f/255.0f}},
        Vertex{{-1.0f, -1.0f, 1.0f}, {76.0f/255.0f, 192.0f/255.0f, 194.0f/255.0f}},
        Vertex{{1.0f, -1.0f, 1.0f}, {56.0f/255.0f, 222.0f/255.0f, 130.0f/255.0f}},
        Vertex{{1.0f, 1.0f, 1.0f}, {234.0f/255.0f, 140.0f/255.0f, 85.0f/255.0f}},
        Vertex{{-1.0f, 1.0f, 1.0f}, {249.0f/255.0f, 220.0f/255.0f, 92.0f/255.0f}},
    };
}

inline std::array<uint16_t, 36> MakeCubeIndices()
{
    return {
        0, 1, 5,
        0, 2, 1,
        0, 3, 2,
        0, 4, 7,
        0, 5, 4,
        0, 7, 3,
        1, 2, 6,
        1, 6, 5,
        3, 6, 2,
        3, 7, 6,
        4, 5, 6,
        4, 6, 7,
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