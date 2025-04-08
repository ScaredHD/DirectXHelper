
cbuffer cbSettings
{

}

Texture2D g_inputA;
Texture2D g_inputB;
RWTexture2D<float4> g_output;

[numthreads(16, 16, 1)]
void CSMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
  g_output[dispatchThreadID.xy] = g_inputA[dispatchThreadID.xy] + g_inputB[dispatchThreadID.xy];
}

