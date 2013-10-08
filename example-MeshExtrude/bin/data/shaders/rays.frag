
#version 150

uniform float exposure = 0.79;
uniform float decay = 0.01;
uniform float density = 0.26;
uniform float weight = 1.22;
const vec2 lightPositionOnScreen = vec2(512, 384);

uniform sampler2DRect rtex;
uniform sampler2DRect otex;
uniform sampler2DRect dtex;

const int NUM_SAMPLES = 30;

in vec4 colorVarying;
in vec2 texCoordVarying;
in vec4 normalVarying;

out vec4 fragColor;

void main()
{   

	vec4 ov = texture(otex, texCoordVarying);

	vec2 deltaTextCoord = vec2( texCoordVarying - lightPositionOnScreen.xy );
	deltaTextCoord *= 1.0 /  float(NUM_SAMPLES) * density;
	float illuminationDecay = 1.0;
	vec2 textCoord;

	for(int i=0; i < NUM_SAMPLES ; i++)
	{
			textCoord -= deltaTextCoord;
			vec4 sample = texture(rtex, texCoordVarying);
			sample *= illuminationDecay * weight;
			fragColor += sample;
			illuminationDecay *= decay;
	}

	vec4 dt = texture(dtex, texCoordVarying);
	vec3 min = vec3(0.9,0.9,0.9);
	//vec3 max = vec3(1.0, 1.0, 1.0) - ((dt.xyz - min) * 10.0);
	vec3 max = vec3((dt.xyz - min) * 10.0);
	fragColor = ((fragColor * exposure * vec4(max, 1.0)) + ov);
	//fragColor = vec4(max, 1.0);
}