#version 450

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec2 fragmentPosition;

layout(push_constant) uniform constants {
	mat4 model;
    vec4 color;
} PushConstants;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
} ubo;

void main() {
    fragmentPosition = inPosition;
    gl_Position = ubo.projection * ubo.view * PushConstants.model * vec4(inPosition, 0.0, 1.0);
}
