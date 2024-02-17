#version 450

layout(location = 0) in vec2 inPosition;

layout(push_constant) uniform constants {
	int penX;
    int penY;
    int screenWidth;
    int screenHeight;
} PushConstants;

vec2 toVulkanNDC(vec2 vertex) {
    return vec2(
        2.0 * (vertex.x / float(PushConstants.screenWidth)) - 1.0,
        -2.0 * (vertex.y / float(PushConstants.screenHeight)) + 1.0
    );
}

void main() {
    vec2 ndcPosition = toVulkanNDC(inPosition);
    float x = ndcPosition.x + 2.0 * float(PushConstants.penX) / float(PushConstants.screenWidth);
    float y = ndcPosition.y + 2.0 * float(PushConstants.penY) / float(PushConstants.screenHeight) - 2.0;

    gl_Position = vec4(x, y, 0.0, 1.0);
}
