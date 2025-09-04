

cbuffer ConstantBuffer : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float time;
}

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput MainVS(VSInput input)
{
    PSInput output;
    output.position = mul(projection, mul(view, float4(input.position, 1.0f)));
    output.color = input.color;
    return output;
}


float4 MainPS(PSInput input) : SV_TARGET
{
    return input.color;
}


