#version 150

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 textureMatrix;
uniform mat4 normalMatrix;

in vec4 position;
in vec4 color;
in vec4 normal;
in vec2 texcoord;

out vec4 vertexColor;
out vec3 vertexNormal;
out vec4 vertexPosition;
out vec2 texCoordVarying;

void main() {
    // Transform vertex position to view space
    vertexPosition = modelViewMatrix * position;
    
    // Transform normal to view space
    vertexNormal = normalize((normalMatrix * normal).xyz);
    
    // Pass through color and texture coordinates
    vertexColor = color;
    texCoordVarying = (textureMatrix * vec4(texcoord.x, texcoord.y, 0, 1)).xy;
    
    // Output final position
    gl_Position = modelViewProjectionMatrix * position;
}
