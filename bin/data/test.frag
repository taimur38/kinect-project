#version 150

uniform sampler2DRect depthData;

in vec2 varyingtexcoord;
in float ftime;
out vec4 outputColor;

void main() 
{

	vec4 depth = texture(depthData, vec2(gl_FragCoord.x, gl_FragCoord.y));

	outputColor = vec4(ftime, depth.g, depth.b, 1.0);

}
