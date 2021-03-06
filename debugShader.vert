#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TextureCoords;

out vec2 UVCoords;

void main()
{
    UVCoords = TextureCoords;
    gl_Position = vec4(Position, 1.0);
}