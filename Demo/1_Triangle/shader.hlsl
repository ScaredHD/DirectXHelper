
cbuffer ConstantBuffer : register(b0)
{
    float time;
}

cbuffer ConstantBufferByDynamicDescriptorHeap : register(b1)
{
    float time2;
}

struct VSOut {
    float4 position : SV_Position;
    float4 color : COLOR;
};


VSOut MainVS(float4 position : POSITION, float4 color : COLOR)
{
    VSOut output;
    output.position = position;
    output.color = float4(sin(color.xyz + time * float3(1.f, 2.f, fmod(time2, 3.f))), 1.f);
    // output.color = color;
    return output;
}

float4 MainPS(VSOut input) : SV_Target
{
    return input.color;
}
