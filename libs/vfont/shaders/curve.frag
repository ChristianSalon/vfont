#version 450

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform constants {
	mat4 model;
    vec4 color;
} PushConstants;

void main() {
    outColor = PushConstants.color;
}
