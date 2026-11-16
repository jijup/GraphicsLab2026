#version 330 core
// A topographic map: color by height, contour lines, and a little shading. Nothing to do here.
in float height;
in vec3 fragPos;

uniform vec3 viewPos;

out vec4 color;

/// Map colors from deep water to snow, by height
vec3 ramp(float h) {
    const int N = 7;
    float stops[N] = float[](-1.0, -0.35, -0.04, 0.0, 0.3, 0.6, 0.9);
    vec3 cols[N] = vec3[](vec3(0.10, 0.22, 0.45), vec3(0.30, 0.52, 0.75), vec3(0.62, 0.80, 0.90),
                          vec3(0.45, 0.66, 0.38), vec3(0.85, 0.80, 0.48), vec3(0.62, 0.43, 0.28),
                          vec3(0.97, 0.97, 0.97));
    if (h <= stops[0]) return cols[0];
    for (int i = 1; i < N; ++i)
        if (h < stops[i]) return mix(cols[i - 1], cols[i], (h - stops[i - 1]) / (stops[i] - stops[i - 1]));
    return cols[N - 1];
}

/// 1 on a contour line, 0 elsewhere. Lines at heights (k + 1/2) spacing, about "width" pixels wide.
float contour(float h, float spacing, float width) {
    float c = h / spacing;
    float d = abs(fract(c) - 0.5) / max(fwidth(c), 1e-5);
    return 1.0 - smoothstep(0.0, width, d);
}

void main() {
    vec3 c = ramp(height);

    // Shading from the triangle's own normal, so the hills look 3D 
    vec3 n = normalize(cross(dFdx(fragPos), dFdy(fragPos)));
    if (n.z < 0.0) n = -n;
    vec3 l = normalize(vec3(-0.5, -0.4, 0.75));
    c *= 0.6 + 0.4 * min(max(dot(n, l), 0.0) / l.z, 1.4);   // flat ground: 1.0

    // Contour lines every 0.1 in height, and a darker one every 0.5
    c = mix(c, c * 0.45, 0.7 * contour(height, 0.1, 1.0));
    c = mix(c, c * 0.25, 0.9 * contour(height, 0.5, 1.4));

    color = vec4(c, 1.0);
}
