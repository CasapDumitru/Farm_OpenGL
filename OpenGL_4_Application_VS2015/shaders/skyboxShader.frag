#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform float luminosity;
uniform float fogDensity;
float computeFog()
{
	 float fragmentDistance = 30;
	 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
		
	 return clamp(fogFactor, 0.0f, 1.0f);
}


void main()
{
	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	
    vec4 fcolor = texture(skybox, textureCoordinates) * luminosity;
	color = mix(fogColor, fcolor, fogFactor);
}
