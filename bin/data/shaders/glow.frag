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
    float pulseIntensity = glowIntensity * (1.0 + 0.3 * sin(time * 3.0));
    
    // Calculate rim lighting (stronger glow at edges)
    vec3 viewDir = normalize(-vertexPosition.xyz);
    float rimFactor = 1.0 - max(dot(vertexNormal, viewDir), 0.0);
    rimFactor = pow(rimFactor, 3.0); // Stronger rim effect
    
    // Enhanced glow effect
    vec3 baseColor = vertexColor.rgb;
    vec3 glowEffect = glowColor * pulseIntensity * (rimFactor + 0.5); // Add base glow
    
    // Add bloom effect with higher intensity
    float bloomFactor = 0.7;
    vec3 bloomColor = mix(baseColor, glowEffect * 1.5, bloomFactor);
    
    // Final color with full opacity
    outputColor = vec4(bloomColor + glowEffect, 1.0);
}
