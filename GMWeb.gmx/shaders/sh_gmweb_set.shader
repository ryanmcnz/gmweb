struct VS_OUTPUT {
    float4 position: SV_POSITION;
};

VS_OUTPUT main() {
    VS_OUTPUT OUT;
    OUT.position = float4(0.0, 0.0, 0.0, 0.0);
    return OUT;
}
//######################_==_YOYO_SHADER_MARKER_==_######################@~struct PS_OUTPUT {
    float4 color_0: SV_TARGET0;
};

PS_OUTPUT main() {
    PS_OUTPUT OUT;
    OUT.color_0 = float4(0.0, 0.0, 0.0, 0.0);
    return OUT;
}
