#version 330

in vec4 Color0;

layout( location = 0 ) out vec4 FragColor;

void main() 
{
	vec3 N;
    N.xy = gl_PointCoord* 2.0 - vec2(1.0);    
    float mag = dot(N.xy, N.xy);
    if (mag > 1.0) {
        discard;
    }

    FragColor = Color0;
}
