// Triangle Vertex Shader
// Simple vertex shader for rendering a triangle

struct VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    // Transform position to clip space
    output.position = float4(input.position, 1.0f);
    
    // Pass through color
    output.color = input.color;
    
    return output;
}