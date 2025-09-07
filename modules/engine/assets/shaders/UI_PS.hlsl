// UI Pixel Shader
// Simple pixel shader for rendering UI elements

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR0;
    nointerpolation uint primitiveID : TEXCOORD0;
};

float4 main(PixelInput input) : SV_TARGET
{
    // Always use the color passed from vertex shader
    return float4(input.color, 1.0f);
}