#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 Position;
out vec3 Normal;
out vec4 Position_lightspace;
//out vec4 shadowCoords;

// Values that stay constant for the whole mesh.
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 lightP;
uniform mat4 lightV;

void main()
{
	Position = vec3(M * vec4(vertexPosition, 1.0));
    Normal = transpose(inverse(mat3(M))) * vertexNormal;
    UV = vertexUV;
    Position_lightspace = lightP * lightV * vec4(Position, 1.0);
    gl_Position = P * V * M * vec4(Position, 1.0);
}