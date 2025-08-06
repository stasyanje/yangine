// Triangle Pixel Shader
// Simple pixel shader for rendering a colored triangle

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(PixelInput input) : SV_TARGET
{
    // Return the interpolated vertex color
    return input.color;
}