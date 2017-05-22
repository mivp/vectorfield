#version 330
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;

uniform mat4 uMV;
uniform mat4 uMVP;
uniform float uScreenHeight = 800;
uniform float uPointScale = 0.1;
uniform vec2 uPointSizeRange = vec2(2, 100);

out vec4 Color0;

void main(void)
{
	gl_Position = uMVP * vec4(inPosition, 1);

	Color0 = inColor;

	vec3 mvPosition = (uMV * vec4(inPosition,1.0)).xyz;
	float projFactor = 2.41; //1.0 / tan(uFOV / 2.0);
	projFactor /= length(mvPosition);
	projFactor *= uScreenHeight / 2.0;
	gl_PointSize = 1000 * uPointScale * projFactor;
	gl_PointSize = max(uPointSizeRange[0], gl_PointSize);
    gl_PointSize = min(uPointSizeRange[1], gl_PointSize);
}
