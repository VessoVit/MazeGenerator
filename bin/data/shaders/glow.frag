#version 150

#ifdef GL_ES
precision mediump float;
#endif

uniform vec3 glowColor;
uniform float glowIntensity;
uniform float time;

in vec4 vertexColor;
in vec3 vertexNormal;
in vec4 vertexPosition;

out vec4 outputColor;

void main() {
    // Base color with simple rim lighting
    vec3 baseColor = vertexColor.rgb;
    vec3 viewDir = normalize(-vertexPosition.xyz);
    float rimFactor = 1.0 - max(dot(vertexNormal, viewDir), 0.0);
    rimFactor = pow(rimFactor, 3.0);
    
    // Simple pulsing effect
    float pulse = 0.8 + 0.2 * sin(time * 2.0);
    
    // Combine effects
    vec3 finalColor = mix(baseColor, glowColor, rimFactor * pulse * glowIntensity);
    
    // Output with full opacity
    outputColor = vec4(finalColor, 1.0);
}
