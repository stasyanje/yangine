cbuffer ShaderConstants : register(b0)
{
    float4x4 model;
    float4x4 viewProjection;
};

struct VertexInput
{
    float3 position : POSITION;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    nointerpolation uint primitiveID : TEXCOORD0;
};

VertexOutput main(VertexInput input, uint vertexID : SV_VertexID)
{
    VertexOutput output;

    float4 world = mul(float4(input.position, 1.0), model);
    output.position = mul(world, viewProjection);
    output.primitiveID = vertexID / 3;

    return output;
}