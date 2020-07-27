#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position;
in vec3 Normal;
in vec4 Position_lightspace;
//in vec4 shadowCoords;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D Texture;
uniform sampler2D depthMap;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform mat4 P;
uniform mat4 V;

void main()
{
	vec3 TexColour = texture(Texture, UV).rgb;
	vec3 n = normalize(Normal);

	//ambient

	vec3 colourA = 0.1 * TexColour;

	//diffuse

	vec3 l = normalize(lightPosition - Position);
	vec3 colourD = max(dot(l, n), 0.0) * vec3(0.3);

	//specular

	vec3 v = normalize(cameraPosition - Position);
	vec3 h = normalize(l + v);
	vec3 colourS = pow(max(dot(n, h), 0.0), 32.0) * vec3(0.3);
	
	//shadow

	vec3 shadowCoords = Position_lightspace.xyz / Position_lightspace.w;
	//vec3 shadowCoords = Position_lightspace.xyz;
	shadowCoords = shadowCoords * 0.5 + 0.5;
	float closestDepth = texture(depthMap, shadowCoords.xy).r;
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(depthMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(depthMap, shadowCoords.xy + vec2(x, y) * texelSize).r;
        	if ((shadowCoords.z - 0.005) > pcfDepth)
			{
				shadow += 1.0;
			} 
		}
	}
	shadow /= 9.0;
	
	color = (colourA + (1.0 - shadow) * (colourD + colourS)) * TexColour;
}