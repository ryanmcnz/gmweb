struct VS_INPUT {
    float4 position: POSITION;
    float2 uv: TEXCOORD0;
};

struct VS_OUTPUT {
    float4 position: SV_POSITION;
    float2 uv: TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT IN) {
    VS_OUTPUT OUT;
    OUT.position = mul(gm_Matrices[MATRIX_WORLD_VIEW_PROJECTION], IN.position);
    OUT.uv = IN.uv;
    return OUT;
}
//######################_==_YOYO_SHADER_MARKER_==_######################@~struct PS_INPUT {
    float2 uv: TEXCOORD0;
};

struct PS_OUTPUT {
    float4 color_0: SV_TARGET0;
};

sampler2D texture_gmweb_surface;

PS_OUTPUT main(PS_INPUT IN) {
    PS_OUTPUT OUT;
    OUT.color_0 = tex2D(texture_gmweb_surface, IN.uv);
    return OUT;
}
