#version 450

layout(location = 0) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform constants {
	mat4 model;
    vec4 color;
} PushConstants;

layout(set = 1, binding = 0) uniform sampler2D fontAtlas;

void main() {
    float distance = texture(fontAtlas, fragUv).r;
    float alpha = distance > 0.5f ? 1.f : 0.f;
    outColor = PushConstants.color * vec4(1.f, 1.f, 1.f, alpha);
}
