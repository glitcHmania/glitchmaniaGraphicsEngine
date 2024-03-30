cbuffer ConstantBuffer2
{
    float4 colors[5];
};

float4 main( uint tid : SV_PrimitiveID ) : SV_TARGET
{
    return colors[tid/8];
}