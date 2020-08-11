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
uniform samplerCube cubeMap;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform mat4 P;
uniform mat4 V;

void main()
{
	vec3 i = normalize(Position - cameraPosition);
	vec3 r = reflect(i, normalize(Normal));

	//vec3 TexColour = texture(Texture, UV).rgb;
	vec3 TexColour = texture(cubeMap, r).rgb;
	vec3 n = normalize(Normal);
	vec3 l = normalize(lightPosition - Position);

	//ambient

	//vec3 colourA = 0.1 * TexColour;
	vec3 colourA = vec3(0.1);

	vec3 colourD, colourS;
	float shadow;

/*	if (dot(l, n) < 0.0) //fragment is within shadow
	{
		//do not bother with specular component
		colourD = vec3(0.0);
		colourS = vec3(0.0);
		shadow = 1.0;

		color = colourA * TexColour;
	}
	else
	{*/
		//diffuse

		//colourD = max(dot(l, n), 0.0) * vec3(0.3);
		colourD = vec3(max(dot(l, n), 0.0));

		//specular

		vec3 v = normalize(cameraPosition - Position);
		vec3 h = normalize(l + v);
		//colourS = pow(max(dot(n, h), 0.0), 32.0) * vec3(0.3);
		colourS = vec3(pow(max(dot(n, h), 0.0), 32.0));

		//shadow

		vec3 shadowCoords = Position_lightspace.xyz / Position_lightspace.w;
		//vec3 shadowCoords = Position_lightspace.xyz;
		shadowCoords = shadowCoords * 0.5 + 0.5;
		float closestDepth = texture(depthMap, shadowCoords.xy).r;
		
		shadow = 0.0;
		//if (shadowCoords.z - 0.0005 > closestDepth) shadow = 1.0;
		vec2 texelSize = 1.0 / textureSize(depthMap, 0);
		//float bias = max(0.05 * (1.0 - dot(n, l)), 0.05);
		//if (shadowCoords.z > closestDepth) shadow = 1.0;
		for (int x = -1; x <= 1; ++x)
		{
			for (int y = -1; y <= 1; ++y)
			{
				float pcfDepth = texture(depthMap, shadowCoords.xy + vec2(x, y) * texelSize).r;
				if ((shadowCoords.z - 0.0005) > pcfDepth)
				{
					shadow += 1.0;
				} 
			}
		}
		shadow /= 9.0;

		//color = (colourA + (1.0 - shadow) * (colourD + colourS)) * TexColour;
		color = ((1.0 - shadow) * colourS) + ((1.0 - shadow) * TexColour) + TexColour * colourA;
	//}
}