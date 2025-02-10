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
    // Calculate pulsing glow intensity with sharper falloff
    float pulseIntensity = glowIntensity * (1.0 + 0.2 * sin(time * 3.0));
    
    // Calculate rim lighting with tighter edge glow
    vec3 viewDir = normalize(-vertexPosition.xyz);
    float rimFactor = 1.0 - max(dot(vertexNormal, viewDir), 0.0);
    rimFactor = pow(rimFactor, 5.0); // Much sharper rim effect
    
    // Calculate distance-based attenuation
    float distanceFromCenter = length(vertexPosition.xyz);
    float attenuationFactor = exp(-distanceFromCenter * 0.1); // Exponential falloff
    
    // Enhanced glow effect with tight radius
    vec3 baseColor = vertexColor.rgb;
    vec3 glowEffect = glowColor * pulseIntensity * rimFactor * attenuationFactor;
    
    // Add localized bloom effect
    float bloomFactor = 0.5;
    vec3 bloomColor = mix(baseColor, glowEffect, bloomFactor * attenuationFactor);
    
    // Final color with opacity based on attenuation
    float alpha = mix(0.0, 1.0, attenuationFactor * rimFactor);
    outputColor = vec4(bloomColor + glowEffect, alpha);
}
