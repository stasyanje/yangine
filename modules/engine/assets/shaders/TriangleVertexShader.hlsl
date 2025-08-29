cbuffer ShaderConstants : register(b0)
{
    float4x4 model;
    float4x4 viewProjection;
};

struct VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    uint instanceID : SV_InstanceID;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;

    float4 world = mul(float4(input.position, 1.0), model);
    output.position = mul(world, viewProjection);
    output.color = input.color;

    return output;
}