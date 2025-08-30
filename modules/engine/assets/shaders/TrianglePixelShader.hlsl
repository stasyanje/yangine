// Triangle Pixel Shader
// Simple pixel shader for rendering a colored triangle

struct PixelInput
{
    float4 position : SV_POSITION;
    nointerpolation uint primitiveID : TEXCOORD0;
};

float4 main(PixelInput input) : SV_TARGET
{
    return float4(
        input.primitiveID % 2 == 0,
        input.primitiveID % 3 == 0,
        input.primitiveID % 4 == 0,
        1.0f
    );
}