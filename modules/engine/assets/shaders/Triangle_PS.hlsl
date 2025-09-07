// Triangle Pixel Shader
// Simple pixel shader for rendering a colored triangle

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR0;
    nointerpolation uint primitiveID : TEXCOORD0;
};

float4 main(PixelInput input) : SV_TARGET
{
    return float4(input.color, 1.0f);
}