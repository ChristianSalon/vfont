#version 450

layout(location = 0) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform constants {
	mat4 model;
    vec4 color;
    bool useSoftEdges;
    float softEdgeMin;
    float softEdgeMax;
} PushConstants;

layout(set = 1, binding = 0) uniform sampler2D fontAtlas;

void main() {
    float distance = texture(fontAtlas, fragUv).r;

    float alpha;
    if(PushConstants.useSoftEdges) {
        // Alpha blending
        alpha = smoothstep(PushConstants.softEdgeMin, PushConstants.softEdgeMax, distance);
    }
    else {
        // Alpha testing
        alpha = distance > 0.5 ? 1.f : 0.f;
    }

    outColor = PushConstants.color * vec4(1.f, 1.f, 1.f, alpha);

    // Prevent z-fighting
    if(alpha < 0.01) {
        discard;
    }
}
