struct Output
{
    float3 color : Color;
    float4 pos : SV_Position;
};

cbuffer ConstantBuffer
{
    matrix transformation;
};

Output main(float3 pos : Position, float3 color : Color)
{
    Output op;
    op.color = color;
    op.pos = mul(float4(pos.x, pos.y, pos.z, 1.0f), transformation);
    return op;
}