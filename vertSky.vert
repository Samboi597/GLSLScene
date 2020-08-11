#version 330 core

layout (location = 0) in vec3 vertexPos;

out vec3 texCoords;

uniform mat4 P;
uniform mat4 V;

void main()
{
    texCoords = vertexPos;
    vec4 pos = P * V * vec4(vertexPos, 1.0);
    gl_Position = pos.xyww;
}