
float4 MainVS(float4 position : POSITION, float4 color : COLOR, out float4 outColor : COLOR) : SV_Position
{
    outColor = color; 
    return position;
}

float4 MainPS(float4 position : SV_Position, float4 color : COLOR) : SV_Target
{
    return color;
}
