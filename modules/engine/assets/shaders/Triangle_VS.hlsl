cbuffer ShaderConstants : register(b0)
{
    float4x4 model;
    float4x4 modelRotated;
    float4x4 viewProjection;
    float time;
    float3 padding;
};

struct VertexInput
{
    float3 position : POSITION;
    float3 color : COLOR0;
};  

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    nointerpolation float3 color : COLOR0;
    nointerpolation uint instanceID : TEXCOORD2;
};

// Helper method to compute an orbiting offset around the Y axis
float3 ComputeOrbitOffset(float radius, float speed, float t)
{
    float angle = t * speed;
    return float3(cos(angle) * radius, 0.0, sin(angle) * radius);
}

// Calculate normal based on vertex ID (cube faces)
float3 GetCubeNormal(uint vertexID)
{
    uint faceIndex = vertexID / 6; // 6 vertices per face
    
    if (faceIndex == 0)      return float3(-1.0,  0.0,  0.0); // Left face
    else if (faceIndex == 1) return float3( 0.0,  0.0, -1.0); // Front face  
    else if (faceIndex == 2) return float3( 0.0, -1.0,  0.0); // Bottom face
    else if (faceIndex == 3) return float3( 0.0,  0.0,  1.0); // Back face
    else if (faceIndex == 4) return float3( 1.0,  0.0,  0.0); // Right face
    else                     return float3( 0.0,  1.0,  0.0); // Top face
}

float3 Distort(float3 position, uint vertexID)
{
    float3 distorted = position;

    float interval = time + vertexID;
    float mult = 0.1 * vertexID;

    distorted.x += sin(interval) * mult;
    distorted.z += cos(interval) * mult;
    distorted.y += -cos(interval) * mult;

    return distorted;
}

float3 Shake(float3 position)
{
    float shakeInterval = 1.5;
    float shakeDuration = 0.4;
    
    float cycleTime = fmod(time, shakeInterval);
    
    if (cycleTime > shakeDuration) {
        return position;
    }

    float shakeIntensity = 0.15;
    float shakeFreq = 150.0;
    
    float t = cycleTime / shakeDuration;
    float envelope = sin(t * 3.14159) * (1.0 - t);
    
    float3 shake = float3(
        sin(time * shakeFreq) * shakeIntensity * envelope,
        sin(time * shakeFreq * 1.3) * shakeIntensity * envelope,
        sin(time * shakeFreq * 0.7) * shakeIntensity * envelope
    );
    
    return position + shake;
}

VertexOutput main(VertexInput input, uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VertexOutput output;

    float4x4 transform = model;
    float3 instanceOffset = float3(0.0, 0.0, 0.0);
    float distance = 5.0;
    float3 color = input.color;

    switch (instanceID) {
    case 3:
        instanceOffset = float3(0.0, 0.0, 0.0);
        transform = modelRotated;
        break;

    case 1:
        instanceOffset = Shake(float3( distance, 0.0, 0.0));
        color.r = 0.0;
        color.b += 1.0 - fmod(time, 1.5) * 0.5 - 0.5;
        break;

    case 2: instanceOffset = Distort(float3( 0.0, 0.0, -distance), vertexID); break;
    case 0: instanceOffset = float3( 0.0, 0.0, distance); break;
    case 4: instanceOffset = float3( 0.0, -distance, 0.0); break;
    case 5: instanceOffset = float3( 0.0, distance, 0.0); break;
    case 6: instanceOffset = ComputeOrbitOffset(8.0, 2.0, time); break;
    default: break;
    }

    // Apply instance offset to vertex position
    float3 worldPos = input.position + instanceOffset;
    float4 world = mul(float4(worldPos, 1.0), transform);
    output.position = mul(world, viewProjection);
    
    // Output world position for lighting
    output.worldPos = world.xyz;
    
    // Calculate and transform normal (assuming uniform scaling)
    float3 normal = GetCubeNormal(vertexID);
    output.normal = mul(normal, (float3x3)transform);
    
    output.color = color;
    output.instanceID = instanceID;

    return output;
}