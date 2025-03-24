#version 450

layout(location = 0) in vec2 fragmentPosition;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform constants {
	mat4 model;
    vec4 color;
    uint lineSegmentsStartIndex;
    uint lineSegmentsCount;
    uint curveSegmentsStartIndex;
    uint curveSegmentsCount;
} PushConstants;

layout(set = 1, binding = 0) buffer Segments {
    vec2 segments[];
};

float rayIntersectsLineSegment(vec2 position, vec2 start, vec2 end) {
    // Check if ray and line segment are parallel
    // Winding should be unchanged
    if(start.y == end.y) {
        return 0.f;
    }

    vec2 directionVector = end - start;
    float directionConstant = directionVector.y > 0 ? 1 : -1;

    // Intersection is at line's start point
    if(position.y == start.y) {
        // Check if line segment is pointing up
        // That means the contour defines a fill
        if(directionConstant > 0.f) {
            if(position.x > start.x) {
                // Ray is on the right of line segment
                return 0.f;
            }
            
            // Return winding of 0.5 based on direction vector
            return 0.5f;
        }
        else {
            if(position.x > start.x) {
                // Ray is on the right of line segment
                return 0.f;
            }
            
            // Return winding of -0.5 based on direction vector
            return -0.5f;
        }
    }
    
    // Intersection is at line's end point
    if(position.y == end.y) {
        // Check if line segment is pointing up
        // That means the contour defines a fill
        if(directionConstant > 0.f) {
            if(position.x > end.x) {
                // Ray is on the right of line segment
                return 0.f;
            }
            
            // Return winding of 0.5 based on direction vector
            return 0.5f;
        }
        else {
            if(position.x > end.x) {
                // Ray is on the right of line segment
                return 0.f;
            }
            
            // Return winding of -0.5 based on direction vector
            return -0.5f;
        }
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
    if(abs(position.x - xIntersection) <= 0.1) {
        return 0.f;
    }
    else if(xIntersection > position.x) {
        return directionConstant;
    }

    return 0.f;
}

float getQuadraticDerivativeWinding(double t, double a, double b) {
    double d = 2 * a * t + b;

    if(d > 0) {
        // Curve is pointing up
       return 1;
    }
    else if(d < 0) {
        // Curve is pointing down
        return -1;
    }

    return 0;
}

float getWindingForQuadraticRoot(double t, vec2 position, vec2 start, vec2 control, vec2 end, double a, double b) {
    float winding = 0;
    
    // X coordinate of intersection
    double xIntersection = (1 - t) * (1 - t) * start.x + 2 * (1 - t) * t * control.x + t * t * end.x;

    // Check if intersection is on curve start or end points
    // If true return winding 0.5 or -0.5
    if(position.y == start.y && xIntersection >= position.x && t >= -0.1 && t <= 0.1) {
        return getQuadraticDerivativeWinding(0, a, b) / 2;
    }
    else if(position.y == end.y && xIntersection >= position.x && t >= 0.9 && t <= 1.1) {
        return getQuadraticDerivativeWinding(1, a, b) / 2;
    }

    // Check if root is in range (0, 1)
    if(t > 0 && t < 1) {

        // Check if intersection is on the right from ray
        if(xIntersection >= position.x) {
            winding += getQuadraticDerivativeWinding(t, a, b);
        }
    }

    return winding;
}

float rayIntersectsCurveSegment(vec2 position, vec2 start, vec2 control, vec2 end) {
    float winding = 0.f;

    // Check if ray is above or below curve
    if(
        (position.y < start.y && position.y < control.y && position.y < end.y) ||
        (position.y > start.y && position.y > control.y && position.y > end.y)
    ) {
        return winding;
    }

    // Quadratic formula coefficients
    double a = start.y - 2 * control.y + end.y;
    double b = 2 * (control.y - start.y);
    double c = start.y - position.y;

    // Curve is actually a line
    if(a == 0) {
        return rayIntersectsLineSegment(position, start, end);
    }

    // Quadratic formula roots
    double sqrtD = sqrt(b * b - 4 * a * c);
    double t0 = (-b + sqrtD) / (2 * a);
    double t1 = (-b - sqrtD) / (2 * a);

    // Compute winding for roots
    winding += getWindingForQuadraticRoot(t0, position, start, control, end, a, b);
    winding += getWindingForQuadraticRoot(t1, position, start, control, end, a, b);

    return winding;
}

void main() {
    float windingNumber = 0;

    // Compute winding for line segments
    for(uint i = PushConstants.lineSegmentsStartIndex; i < PushConstants.lineSegmentsStartIndex + 2 * PushConstants.lineSegmentsCount; i += 2) {
        windingNumber += rayIntersectsLineSegment(fragmentPosition, segments[i], segments[i + 1]);
    }
    
    // Compute winding for curve segments
    for(uint i = PushConstants.curveSegmentsStartIndex; i < PushConstants.curveSegmentsStartIndex + 3 * PushConstants.curveSegmentsCount; i += 3) {
        windingNumber += rayIntersectsCurveSegment(fragmentPosition, segments[i], segments[i + 1], segments[i + 2]);
    }

    if(windingNumber != 0) {
        outColor = PushConstants.color;
    }
    else {
        discard;
    }
}
