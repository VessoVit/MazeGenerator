#version 150

uniform vec3 glowColor;
uniform float glowIntensity;
uniform float time;

in vec4 vertexColor;
in vec3 vertexNormal;
in vec4 vertexPosition;

out vec4 outputColor;

void main() {
    // Calculate pulsing glow intensity
    float pulseIntensity = glowIntensity * (1.0 + 0.2 * sin(time * 2.0));
    
    // Calculate rim lighting (stronger glow at edges)
    vec3 viewDir = normalize(-vertexPosition.xyz);
    float rimFactor = 1.0 - max(dot(vertexNormal, viewDir), 0.0);
    rimFactor = pow(rimFactor, 2.0);
    
    // Combine base color with glow
    vec3 baseColor = vertexColor.rgb;
    vec3 glowEffect = glowColor * pulseIntensity * rimFactor;
    
    // Add bloom effect
    float bloomFactor = 0.5;
    vec3 bloomColor = mix(baseColor, glowEffect, bloomFactor);
    
    // Final color
    outputColor = vec4(bloomColor + glowEffect, vertexColor.a);
}
