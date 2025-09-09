// Triangle Pixel Shader
// Simple pixel shader for rendering a colored triangle

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR0;
    nointerpolation uint instanceID : TEXCOORD0;
};

float4 main(PixelInput input) : SV_TARGET
{
    float3 color = input.color;

    color.r = input.instanceID == 2; // red for south
    color.b = input.instanceID == 3; // blue for north

    return float4(color, 1.0f);
}