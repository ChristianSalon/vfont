#version 450

layout(triangles, equal_spacing, ccw) in;

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
} ubo;

bool isTriangleCCW(vec2 a, vec2 b, vec2 c) {
   float determinant = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
   return determinant > 0;
}

// Evaluate quadratic bezier curve
vec2 bezier(float t, vec2 start, vec2 control, vec2 end) {
	return (1.f - t) * (1.f - t) * start + 2 * t * (1.f - t) * control + t * t * end;
}

void main() {
	// Barycentric coordinates
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	// Quadratic bezier curve control points
	vec2 start = gl_in[0].gl_Position.xy;
	vec2 control = gl_in[1].gl_Position.xy;
	vec2 end = gl_in[2].gl_Position.xy;

	vec2 position;
	if(isTriangleCCW(start, control, end)) {
		if(u == 0.f) {
			// Edge from curve control point to end point
			position = bezier(0.5 + (w / 2.f), start, control, end);
		}
		else if (w == 0.f) {
			// Edge from curve start point to control point
			position = bezier(v / 2.f, start, control, end);
		}
		else {
			position = u * start + v * control + w * end;
		}
	}
	else {
		if(v == 0.f) {
			position = bezier(w, start, control, end);
		}
		else {
			position = control;
		}
	}

	gl_Position = ubo.projection * ubo.view * vec4(position, 0.f, 1.f);
}
