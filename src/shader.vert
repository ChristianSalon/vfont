#version 450

layout(location = 0) in vec2 inPosition;

layout(push_constant) uniform constants {
	int x;
    int y;
    int windowWidth;
    int windowHeight;
} PushConstants;

// Convert from freetype 1/64 pixel unit to pixels
vec2 toPixelUnits(vec2 vertex) {
    return vertex / 64.0;
}

vec2 toVulkanNDC(vec2 vertex) {
    return vec2(
        2.0 * (vertex.x / float(PushConstants.windowWidth)) - 1.0,
        -2.0 * (vertex.y / float(PushConstants.windowHeight)) + 1.0
    );
}

void main() {
    vec2 ndcPosition = toVulkanNDC(toPixelUnits(inPosition));
    float x = ndcPosition.x + 2.0 * float(PushConstants.x) / float(PushConstants.windowWidth);
    float y = ndcPosition.y + 2.0 * float(PushConstants.y) / float(PushConstants.windowHeight) - 2.0;

    gl_Position = vec4(x, y, 0.0, 1.0);
}
