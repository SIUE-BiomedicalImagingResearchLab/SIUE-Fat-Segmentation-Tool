vec4 colormap(float x) {
    return vec4(clamp(x, 0.0, 1.0), clamp(x, 0.0, 1.0), clamp(x, 0.0, 1.0), 1.0);
}
