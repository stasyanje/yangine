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
    nointerpolation uint instanceID : TEXCOORD0;
};

VertexOutput main(VertexInput input, uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VertexOutput output;

    float3 instanceOffset = float3(0.0, 0.0, 0.0);
    float distance = 5.0;

    if (instanceID == 0)      instanceOffset = float3(-distance, 0.0, 0.0);
    else if (instanceID == 1) instanceOffset = float3( distance, 0.0, 0.0);
    else if (instanceID == 2) instanceOffset = float3( 0.0, 0.0, -distance);
    else if (instanceID == 3) instanceOffset = float3( 0.0, 0.0, distance);

    // Apply instance offset to vertex position
    float3 worldPos = input.position + instanceOffset;
    float4 world = mul(float4(worldPos, 1.0), model);
    output.position = mul(world, viewProjection);
    output.color = input.color;
    output.instanceID = instanceID;

    return output;
}