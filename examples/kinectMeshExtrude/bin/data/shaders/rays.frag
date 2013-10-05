
#version 150

uniform float exposure = 0.79;
uniform float decay = 0.01;
uniform float density = 0.26;
uniform float weight = 1.22;
uniform vec2 lightPositionOnScreen;

uniform sampler2DRect rtex;
uniform sampler2DRect otex;

const int NUM_SAMPLES = 30;

in vec4 colorVarying;
in vec2 texCoordVarying;
in vec4 normalVarying;

out vec4 fragColor;

void main()
{   

	vec4 ov = texture(otex, texCoordVarying);

	vec2 convertedTexCoordVarying = texCoordVarying;
	vec2 deltaTextCoord = vec2( texCoordVarying - lightPositionOnScreen.xy );
	vec2 textCoord = convertedTexCoordVarying;
	deltaTextCoord *= 1.0 /  float(NUM_SAMPLES) * density;
	float illuminationDecay = 1.0;


	for(int i=0; i < NUM_SAMPLES ; i++)
	{
			textCoord -= deltaTextCoord;
			vec4 sample = texture(rtex, textCoord);
			sample *= illuminationDecay * weight;
			fragColor += sample;
			illuminationDecay *= decay;
	}

	vec4 inv = vec4(1, 1, 1, 1);
	fragColor = (fragColor * exposure) + ov;
	//fragColor = texture(rtex, textCoord);
}