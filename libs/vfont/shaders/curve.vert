#version 450

layout(location = 0) in vec2 inPosition;

layout(push_constant) uniform constants {
	mat4 model;
    vec4 color;
} PushConstants;

void main() {
    gl_Position = PushConstants.model * vec4(inPosition, 0.0, 1.0);
}
