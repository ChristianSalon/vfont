#version 450

layout(location = 0) in vec2 fragmentPosition;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform constants {
	mat4 model;
    vec4 color;
    uint lineSegmentsStartIndex;
    uint lineSegmentsCount;
} PushConstants;

struct LineSegment {
    vec2 start;
    vec2 end;
};

layout(set = 1, binding = 0) buffer LineSegments {
    LineSegment lineSegments[];
};

float rayIntersectsLineSegment(vec2 position, vec2 start, vec2 end) {
    // Check if ray and line segment are parallel
    // Winding should be unchanged
    if(start.y == end.y) {
        return 0.f;
    }

    vec2 directionVector = normalize(end - start);
    float directionConstant = directionVector.y > 0 ? 1 : -1;

    // Intersection is at line's start or end point
    // Return winding of 0.5 or -0.5 depending on direction vector
    if(position.y == start.y || position.y == end.y) {
        return directionConstant / 2.f;
    }

    // Ensure line's start point is below end point
    if(start.y > end.y) {
        vec2 temp = start;
        start = end;
        end = temp;
    }

    // Ray is above or below line
    if(position.y < start.y || position.y > end.y) {
        return 0.f;
    }

    // Check if intersection is to the right of ray
    // If true return winding of 1 or -1 depending on direction vector
    float xIntersection = start.x + (position.y - start.y) * (end.x - start.x) / (end.y - start.y);
    if(xIntersection >= position.x) {
        return directionConstant;
    }

    return 0.f;
}

void main() {
    float windingNumber = 0;
    for(uint i = PushConstants.lineSegmentsStartIndex; i < PushConstants.lineSegmentsStartIndex + PushConstants.lineSegmentsCount; i++) {
        LineSegment segment = lineSegments[i];
        windingNumber += rayIntersectsLineSegment(fragmentPosition, segment.start, segment.end);
    }

    if(windingNumber != 0) {
        outColor = PushConstants.color;
    }
    else {
        discard;
    }
}
