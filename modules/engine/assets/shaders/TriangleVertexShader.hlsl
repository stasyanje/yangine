// Triangle Vertex Shader
// Animated vertex shader for rendering multiple triangles

cbuffer ShaderConstants : register(b0)
{
    float2 mousePos;
    float time;
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

float4 hoveringTriangles(VertexInput input)
{
    // Calculate unique animation for each triangle instance
    float instanceFloat = float(input.instanceID);
    float animationPhase = instanceFloat * 0.628f; // 2*PI/10 for varied phases
    
    // Create circular motion with different speeds
    float radius = 0.3f + (instanceFloat / 100.0f) * 0.4f;
    float speed = 1.0f + (instanceFloat / 50.0f);
    float angle = time * speed + animationPhase;
    
    float2 circularOffset = float2(
        cos(angle) * radius,
        sin(angle) * radius
    );

    // Apply animation offset to vertex position
    float3 animatedPosition = input.position * 5.0 + float3(mousePos * circularOffset, 0.0f);

    return float4(animatedPosition, 1.0f);
}

float4 relaxedVibe(VertexInput input)
{
    // Modify color based on instance ID for variety

    // Calculate unique animation for each triangle instance
    float instanceFloat = float(input.instanceID);
    float animationPhase = instanceFloat * 0.628f; // 2*PI/10 for varied phases
    float colorShift = instanceFloat / 100.0f;

    return float4(
        input.color.r * (0.5f + colorShift),
        input.color.g * (0.8f + sin(time + animationPhase) * 0.2f),
        input.color.b * (0.7f + cos(time + animationPhase) * 0.3f),
        input.color.a
    );
}

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.position = hoveringTriangles(input);
    output.color = relaxedVibe(input);
    return output;
}