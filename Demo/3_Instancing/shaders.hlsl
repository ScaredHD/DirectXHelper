

cbuffer ConstantBuffer : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float time;
}

struct InstanceData
{
    float4x4 world;
    float4 color;
};

StructuredBuffer<InstanceData> g_instanceBuffer : register(t0);

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    uint instanceID : SV_InstanceID;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput MainVS(VSInput input)
{
    PSInput output;
    output.position = mul(projection, mul(view, mul(g_instanceBuffer[input.instanceID].world, float4(input.position, 1.0f))));
    output.color = g_instanceBuffer[input.instanceID].color;
    return output;
}


float4 MainPS(PSInput input) : SV_TARGET
{
    return input.color;
}


