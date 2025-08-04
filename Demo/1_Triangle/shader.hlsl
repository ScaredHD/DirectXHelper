
cbuffer ConstantBuffer : register(b0)
{
    float time;
}

struct VSOut {
    float4 position : SV_Position;
    float4 color : COLOR;
};


VSOut MainVS(float4 position : POSITION, float4 color : COLOR)
{
    VSOut output;
    output.position = position;
    output.color = float4(sin(color.xyz + time * float3(1.f, 2.f, 3.f)), 1.f);
    return output;
}

float4 MainPS(VSOut input) : SV_Target
{
    return input.color;
}
