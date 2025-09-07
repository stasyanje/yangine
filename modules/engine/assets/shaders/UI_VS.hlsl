cbuffer ShaderConstants : register(b0)
{
    float4x4 model;
    float4x4 viewProjection;
};

struct VertexInput
{
    float3 position : POSITION;
    float3 color : COLOR0;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR0;
    nointerpolation uint primitiveID : TEXCOORD0;
};

VertexOutput main(VertexInput input, uint vertexID : SV_VertexID)
{
    VertexOutput output;
    
    // For UI rendering, use position directly as screen coordinates
    // Convert from normalized device coordinates [-1,1] to screen space
    output.position = float4(input.position.xy, 0.0, 1.0);
    output.color = float3(1.0, 0.0, 0.0); // Red color for UI triangle
    output.primitiveID = vertexID / 3;

    return output;
}