#version 150

uniform sampler2DRect tex0;
uniform float time;

in vec2 varyingtexcoord;
out vec4 outputColor;

void main() 
{

	vec4 depth = texture(tex0, varyingtexcoord);

	float d = depth.x;

	//outputColor = vec4(sin(ftime), depth.g, depth.b, 1.0);

	float windowWidth = 1920;
    float windowHeight = 1080;

    float r = gl_FragCoord.x / windowWidth;
    float g = gl_FragCoord.y / windowHeight;
    float b = 1.0;
    float a = 1.0;

	//outputColor = vec4(r, g, b, a);
	outputColor = vec4(depth.x, depth.y, depth.z, 255);
}
