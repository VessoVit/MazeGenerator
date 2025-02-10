#version 150

uniform mat4 modelViewProjectionMatrix;
in vec4 position;
in vec4 color;
in vec4 normal;

out vec4 vertexColor;
out vec3 vertexNormal;
out vec4 vertexPosition;

void main() {
    vertexColor = color;
    vertexNormal = normal.xyz;
    vertexPosition = position;
    gl_Position = modelViewProjectionMatrix * position;
}
