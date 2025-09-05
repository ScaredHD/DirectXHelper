

cbuffer ConstantBuffer : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 lightColor;
    float time;
    float3 lightDir;
    // float padding (automatically added to align to 16 bytes)
    float3 ambient;
}

struct InstanceData
{
    float4x4 world;
    float4x4 invWorld;
    float4 albedo;
};

StructuredBuffer<InstanceData> g_instanceBuffer : register(t0);

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;  // not used in this demo
    float3 normal : NORMAL;
    uint instanceID : SV_InstanceID;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 albedo : ALBEDO;
    float3 normal : NORMAL;
};

PSInput MainVS(VSInput input)
{
    PSInput output;
    float4x4 world = g_instanceBuffer[input.instanceID].world;
    output.position = mul(projection, mul(view, mul(world, float4(input.position, 1.0f))));

    output.albedo = g_instanceBuffer[input.instanceID].albedo;

    float4x4 invWorld = g_instanceBuffer[input.instanceID].invWorld;
    float4x4 normalMat = transpose(invWorld);
    output.normal = mul(normalMat, float4(input.normal, 0.0f)).xyz;

    return output;
}


float4 MainPS(PSInput input) : SV_TARGET
{
    float3 l = normalize(lightDir);
    float3 n = normalize(input.normal);
    float4 albedo = input.albedo;
    float n_l = max(dot(n, l), 0.0);
    float4 color = float4(ambient, 1.f) +  albedo * n_l * float4(lightColor, 1.0f);
    return color;
}


