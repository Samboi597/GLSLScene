#version 330 core

out vec3 color;

in vec2 UVCoords;

uniform sampler2D depthMap;

void main()
{
    float depthValue = texture(depthMap, UVCoords).r;

    float nearPlane = 1.0;
    float farPlane = 20.0;
    float perspDepth = (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - (depthValue * 2.0 - 1.0) * (farPlane - nearPlane));
    color = vec3(perspDepth / farPlane);
}