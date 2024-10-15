#version 450

layout(vertices = 3) out;

bool isTriangleCCW(vec2 a, vec2 b, vec2 c) {
   float determinant = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
   return determinant > 0;
}

void main() {
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	// Quadratic bezier curve control points
	vec2 start = gl_in[0].gl_Position.xy;
	vec2 control = gl_in[1].gl_Position.xy;
	vec2 end = gl_in[2].gl_Position.xy;

	float maxCurveLength = distance(start, control) + distance(control, end);
	vec2 lineMidpoint = abs(end - start) / 2.f;
	float distance = distance(lineMidpoint, control);
	float tessellationLevel = sqrt(distance) * maxCurveLength;

	// The curve is on the part of the contour that defines a filled outline
	if(isTriangleCCW(start, control, end)) {
		gl_TessLevelInner[0] = 1.f;
		gl_TessLevelOuter[0] = tessellationLevel / 2.f;
		gl_TessLevelOuter[1] = 1.f;
		gl_TessLevelOuter[2] = tessellationLevel / 2.f;
	}
	// The curve is on the part of the contour that defines a hole
	else {
		gl_TessLevelInner[0] = 1.f;
		gl_TessLevelOuter[0] = 1.f;
		gl_TessLevelOuter[1] = tessellationLevel;
		gl_TessLevelOuter[2] = 1.f;
	}
}
