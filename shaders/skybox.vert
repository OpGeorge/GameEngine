#version 450

layout(location = 0) out vec3 fragDirection;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    vec4 pointLights[10 * 2]; // unused, but included to match layout
    int numLights;
} ubo;

// A hardcoded cube (or vertex buffer)
vec3 positions[36] = vec3[](
    vec3(-1, -1, -1), vec3(1, -1, -1), vec3(1,  1, -1),
    vec3(1,  1, -1), vec3(-1,  1, -1), vec3(-1, -1, -1),

    vec3(-1, -1, 1), vec3(1, -1, 1), vec3(1,  1, 1),
    vec3(1,  1, 1), vec3(-1,  1, 1), vec3(-1, -1, 1),

    vec3(-1,  1, 1), vec3(-1,  1, -1), vec3(-1, -1, -1),
    vec3(-1, -1, -1), vec3(-1, -1, 1), vec3(-1,  1, 1),

    vec3(1,  1, 1), vec3(1,  1, -1), vec3(1, -1, -1),
    vec3(1, -1, -1), vec3(1, -1, 1), vec3(1,  1, 1),

    vec3(-1, -1, -1), vec3(1, -1, -1), vec3(1, -1, 1),
    vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, -1, -1),

    vec3(-1, 1, -1), vec3(1, 1, -1), vec3(1, 1, 1),
    vec3(1, 1, 1), vec3(-1, 1, 1), vec3(-1, 1, -1)
);

void main() {
    fragDirection = positions[gl_VertexIndex];

    mat4 viewRotOnly = mat4(mat3(ubo.view)); // remove translation
    vec4 clipPos = ubo.projection * viewRotOnly * vec4(fragDirection, 1.0);
    gl_Position = clipPos.xyww; // force depth = 1.0 for background
}