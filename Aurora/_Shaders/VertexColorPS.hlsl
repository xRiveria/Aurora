struct VertexToPixel
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main (VertexToPixel PSIn) : SV_TARGET
{
    return PSIn.color;
}