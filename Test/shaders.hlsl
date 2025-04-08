
void VSMain(
  float3 pos : POSITION,
  float3 normal : NORMAL,
  float4 color : COLOR,
  out float4 posH : SV_Position,
  out float4 outColor : COLOR2
) 
{
  posH = float4(pos, 1.0f);
  outColor = color;
}

float4 PSMain(float4 posH : SV_Position, float4 color : COLOR2) : SV_Target
{
  return color;
}