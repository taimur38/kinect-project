#version 150

uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec4 color;
in vec4 normal;
in vec2 texcoord;

in float time;

out vec2 varyingtexcoord;
out float ftime;

void main() {

	varyingtexcoord = texcoord;
	ftime = time;

	gl_Position = modelViewProjectionMatrix * position;
}
