// Triangle Pixel Shader
// Pixel shader with basic Phong lighting

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    nointerpolation float3 color : COLOR0;
    nointerpolation uint instanceID : TEXCOORD2;
};

float4 main(PixelInput input) : SV_TARGET
{
    float3 color = input.color;

    switch (input.instanceID) {
    case 1: break;
    case 2: color.r = 1.0; break;
    case 3: color.b = 1.0; break;
    case 6: color.rgb = 1.0; break;
    default: color.rb = 0.0; break;
    }

    return float4(color, 1.0);
}